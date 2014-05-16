#include "superupl.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

SuperUpl::SuperUpl(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp SuperUpl::urlPattern() const {
    return QRegExp("http(s|)://(www.|)superupl.com/\\w+", Qt::CaseInsensitive);
}

bool SuperUpl::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void SuperUpl::login(const QString &username, const QString &password) {
    QString data = QString("op=login&redirect=&login=%1&password=%2&x=0&y=0").arg(username).arg(password);
    QUrl url("http://superupl.com/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SuperUpl::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (statusCode) {
    case 302:
    case 200:
    case 201:
        m_connections = 0;
        emit loggedIn(true);
        break;
    default:
        m_connections = 1;
        emit loggedIn(false);
        break;
    }

    reply->deleteLater();
}

void SuperUpl::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SuperUpl::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+.superupl.com:\\d+/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("File Not Found")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("fname\" value=\"", 1, 1).section('"', 0, 0);

            if (fileName.isEmpty()) {
                emit urlChecked(false);
            }
            else {
                emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
            }
        }
    }

    reply->deleteLater();
}

void SuperUpl::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_url = webUrl;
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SuperUpl::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.superupl.com:\\d+/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else if (!redirect.isEmpty()) {
        this->getDownloadRequest(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QNetworkRequest request;
            request.setUrl(QUrl(re.cap()));
            emit downloadRequestReady(request);
        }
        else if (response.contains("File Not Found")) {
            emit error(NotFound);
        }
        else {
            m_fileId = response.section("id\" value=\"", 1, 1).section('"', 0, 0);
            m_fileName = response.section("fname\" value=\"", 1, 1).section('"', 0, 0);
            
            if ((m_fileId.isEmpty()) || (m_fileName.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                this->getWaitTime();
            }
        }
    }

    reply->deleteLater();
}

void SuperUpl::getWaitTime() {
    QString data = QString("op=download1&id=%1&fname=%2&method_free=FREE").arg(m_fileId).arg(m_fileName);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SuperUpl::checkWaitTime() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    int mins = 0;
    int secs = 0;

    if (response.contains("You have to wait")) {
        mins = response.section("You have to wait ", 1, 1).section(" minutes", 0, 0).toInt();
        secs = response.section(" seconds before your next download", 0, 0).section(' ', 1, 1).toInt();
        this->startWait((mins * 60000) + (secs + 1000));
        this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
    }
    else if (response.contains("You can download files up to ")) {
        emit error(TrafficExceeded);
    }
    else {
        secs = response.section(QRegExp("countdown_str\">Wait <span id=\"\\w+\">"), 1, 1).section('<', 0, 0).toInt();
        m_rand = response.section("rand\" value=\"", 1, 1).section('"', 0, 0);
        m_captchaKey = response.section("http://www.google.com/recaptcha/api/challenge?k=", 1, 1).section('"', 0, 0);

        if ((secs <= 0) || (m_rand.isEmpty()) || (m_captchaKey.isEmpty())) {
            emit error(UnknownError);
        }
        else {
            this->startWait(secs * 1000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
        }
    }

    reply->deleteLater();
}

void SuperUpl::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void SuperUpl::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("op=download2&id=%1&fname=%2&rand=%3&method_free=FREE&down_direct=1&recaptcha_challenge_field=%4&recaptcha_response_field=%5").arg(m_fileId).arg(m_fileName).arg(m_rand).arg(challenge).arg(response);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SuperUpl::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.superupl.com:\\d+/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QNetworkRequest request;
            request.setUrl(QUrl(re.cap()));
            emit downloadRequestReady(request);
        }
        else if (response.contains("Wrong captcha")) {
            emit error(CaptchaError);
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void SuperUpl::startWait(int msecs) {
    if (msecs > 60000) {
        emit statusChanged(LongWait);
    }
    else {
        emit statusChanged(ShortWait);
    }

    emit waiting(msecs);
    m_waitTime = msecs;
    m_waitTimer->start(1000);
}

void SuperUpl::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void SuperUpl::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool SuperUpl::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(superupl, SuperUpl)
