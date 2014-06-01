#include "ultramegabit.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

Ultramegabit::Ultramegabit(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Ultramegabit::urlPattern() const {
    return QRegExp("http(s|)://(www.|)ultramegabit.com/file/details/\\w+", Qt::CaseInsensitive);
}

bool Ultramegabit::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Ultramegabit::login(const QString &username, const QString &password) {
    QString data = QString("username=%1&password=%2&submit=Login").arg(username).arg(password);
    QUrl url("http://ultramegabit.com/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Ultramegabit::checkLogin() {
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

void Ultramegabit::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Ultramegabit::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+.ultramegabit.com/files/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("<title>ULTRAMEGABIT.COM -", 1, 1).section('<', 0, 0).trimmed();        
        emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
    }

    reply->deleteLater();
}

void Ultramegabit::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Ultramegabit::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://storage\\d+.ultramegabit.com/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }

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
        else {
            m_captchaKey = response.section("www.google.com/recaptcha/api/challenge?k=", 1, 1).section('"', 0, 0);
            m_fileId = response.section("encode\" value=\"", 1, 1).section('"', 0, 0);
            m_token = response.section("csrf_token\" value=\"", 1, 1).section('"', 0, 0);

            if ((m_captchaKey.isEmpty()) || (m_fileId.isEmpty()) || (m_token.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                this->startWait(30000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
            }
        }
    }

    reply->deleteLater();
}

void Ultramegabit::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void Ultramegabit::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QUrl url("https://ultramegabit.com/file/download");
    QString data = QString("csrf_token=%1&recaptcha_challenge_field=%2&recaptcha_response_field=%3&encode=%4").arg(m_token).arg(challenge).arg(response).arg(m_fileId);
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Ultramegabit::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://storage\\d+.ultramegabit.com/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toString();
    }
    
    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else if (redirect.contains("delay")) {
        this->startWait(600000);
        this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QNetworkRequest request;
            request.setUrl(QUrl(re.cap()));
            emit downloadRequestReady(request);
        }
        else if (response.contains("check_recaptcha")) {
            emit error(CaptchaError);
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void Ultramegabit::startWait(int msecs) {
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

void Ultramegabit::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Ultramegabit::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Ultramegabit::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(ultramegabit, Ultramegabit)
