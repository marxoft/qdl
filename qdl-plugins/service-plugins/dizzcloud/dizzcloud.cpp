#include "dizzcloud.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

Dizzcloud::Dizzcloud(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Dizzcloud::urlPattern() const {
    return QRegExp("http(s|)://(www.|)dizzcloud.com/dl/\\w+", Qt::CaseInsensitive);
}

bool Dizzcloud::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Dizzcloud::login(const QString &username, const QString &password) {
    QString data = QString("email=%1&pass=%2").arg(username).arg(password);
    QUrl url("http://dizzcloud.com/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Dizzcloud::checkLogin() {
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

void Dizzcloud::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Dizzcloud::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+.cloudstoreservice.net/[^\"']+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("class=\"name\">", 1, 1).section("<", 0, 0).trimmed();

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void Dizzcloud::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Dizzcloud::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.cloudstoreservice.net/[^\"']+");
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
        else if (response.contains("Next free download from your ip will be available in")) {
            int mins = response.section("Next free download from your ip will be available in <b>", 1, 1).section("minutes", 0, 0).trimmed().toInt();

            if (mins > 0) {
                this->startWait(mins * 60000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
            }
            else {
                emit error(UnknownError);
            }
        }
        else {
            m_fileId = response.section("getJSON(\"/dl/", 1, 1).section('"', 0, 0);
            m_fileGroup = response.section("'filegroup' : '", 1, 1).section('\'', 0, 0);
            m_h = response.section("'h' : '", 1, 1).section('\'', 0, 0);
            m_captchaKey = response.section("google.com/recaptcha/api/challenge?k=", 1, 1).section('"', 0, 0);

            if ((m_fileId.isEmpty()) || (m_fileGroup.isEmpty()) || (m_h.isEmpty()) || (m_captchaKey.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                this->startWait(60000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
            }
        }
    }

    reply->deleteLater();
}

void Dizzcloud::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void Dizzcloud::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("type=recaptcha&challenge=%1&capture=%2&filegroup=%3&h=%4").arg(challenge).arg(response).arg(m_fileGroup).arg(m_h);
    QUrl url("http://dizzcloud.com/" + m_fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Referer", "http://dizzcloud.net/dl/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Dizzcloud::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.cloudstoreservice.net/[^\"']+");
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
        else if (response.contains("Incorrect captcha")) {
            emit error(CaptchaError);
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void Dizzcloud::startWait(int msecs) {
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

void Dizzcloud::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Dizzcloud::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Dizzcloud::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(dizzcloud, Dizzcloud)
