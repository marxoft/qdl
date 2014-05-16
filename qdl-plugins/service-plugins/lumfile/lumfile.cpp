#include "lumfile.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

Lumfile::Lumfile(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Lumfile::urlPattern() const {
    return QRegExp("http(s|)://(www.|)lumfile.com/\\w+", Qt::CaseInsensitive);
}

bool Lumfile::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Lumfile::login(const QString &username, const QString &password) {
    QString data = QString("op=login&login=%1&password=%2").arg(username).arg(password);
    QUrl url("http://lumfile.com");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Lumfile::checkLogin() {
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

void Lumfile::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Lumfile::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w\\d+.lumfile.com:\\d+/d/[^'\"]+");

    if (!redirect.isEmpty()) {
	if (re.indexIn(redirect) == 0) {
	    emit urlChecked(true, reply->request().url(), this->serviceName(), redirect.section('/', -1));
	}
	else { 
            this->checkUrl(QUrl(redirect));
	}
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

void Lumfile::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Lumfile::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w\\d+.lumfile.com:\\d+/d/[^'\"]+");
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

void Lumfile::getWaitTime() {
    QUrl url("http://lumfile.com/" + m_fileId);
    QString data = QString("op=download1&usr_login=&id=%1&fname=%2&referer=&method_free=Download+slow+speed").arg(m_fileId).arg(m_fileName);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", "http://lumfile.com/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Lumfile::checkWaitTime() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    if (response.contains("You have to wait")) {
        int mins = response.section("You have to wait ", 1, 1).section(" minutes", 0, 0).toInt();
        int secs = response.section(" seconds before your next download", 0, 0).section(' ', 1, 1).toInt();
        this->startWait((mins * 60000) + (secs + 1000));
        this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
    }
    else if (response.contains("Type the two words")) {
        m_rand = response.section("rand\" value=\"", 1, 1).section('"', 0, 0);
        m_captchaKey = response.section("http://www.google.com/recaptcha/api/challenge?k=", 1, 1).section('"', 0, 0);

        if ((m_rand.isEmpty()) || (m_captchaKey.isEmpty())) {
            emit error(UnknownError);
        }
        else {
            this->startWait(60000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
        }
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void Lumfile::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void Lumfile::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QUrl url("http://lumfile.com/" + m_fileId);
    QString data = QString("op=download2&id=%1&method_free=Download+slow+speed&down_script=1&recaptcha_challenge_field=%2&recaptcha_response_field=%3&referer=%4&rand=%5").arg(m_fileId).arg(challenge).arg(response).arg("http://lumfile.com" + m_fileId).arg(m_rand);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", "http://Lumfile.com/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Lumfile::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w\\d+.lumfile.com:\\d+/d/[^'\"]+");
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
        else if (response.contains("Incorrect, please try again")) {
            m_captchaKey = response.section("rand\" value=\"", 1, 1).section('"', 0, 0);

            if (m_captchaKey.isEmpty()) {
                emit error(UnknownError);
            }
            else {
                this->startWait(60000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
            }
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void Lumfile::startWait(int msecs) {
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

void Lumfile::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Lumfile::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Lumfile::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(lumfile, Lumfile)
