#include "shareonline.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

ShareOnline::ShareOnline(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_dlWaitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp ShareOnline::urlPattern() const {
    return QRegExp("http(s|)://(www.|)share-online.biz/dl/\\w+", Qt::CaseInsensitive);
}

bool ShareOnline::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void ShareOnline::login(const QString &username, const QString &password) {
    QString data = QString("user=%1&pass=%2").arg(username).arg(password);
    QUrl url("https://www.share-online.biz/user/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ShareOnline::checkLogin() {
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
        if (reply->readAll().contains("Login data invalid")) {
            m_connections = 1;
            emit loggedIn(false);
        }
        else {
            m_connections = 0;
            emit loggedIn(true);
        }

        break;
    default:
        m_connections = 1;
        emit loggedIn(false);
        break;
    }

    reply->deleteLater();
}

void ShareOnline::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ShareOnline::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://dlw\\d+-\\d.share-online.biz/fl\\?fr=[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("file is not available")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = reply->request().url().toString().section('/', -1);

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

void ShareOnline::getDownloadRequest(const QUrl &webUrl) {
    if (!m_downloadUrl.isEmpty()) {
        emit downloadRequestReady(QNetworkRequest(m_downloadUrl));
        return;
    }

    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ShareOnline::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://dlw\\d+-\\d.share-online.biz/fl\\?fr=[^'\"]+");
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
            m_fileId = reply->request().url().toString().section('/', -1);

            if (m_fileId.isEmpty()) {
                emit error(UnknownError);
            }
            else {
                this->getCaptchaPage();
            }
        }
    }

    reply->deleteLater();
}

void ShareOnline::getCaptchaPage() {
    QUrl url("http://www.share-online.biz/dl/" + m_fileId + "/free/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", "http://www.share-online.biz/dl/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, "dl_free=1&choice=free");
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ShareOnline::checkCaptchaPage() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    m_captchaId = QByteArray::fromBase64(response.section("var dl=\"", 1, 1).section('"', 0, 0).toUtf8()).mid(5);
    m_captchaUrl = response.section("var url='", 1, 1).section('\'', 0, 0).replace("///", "/free/captcha/");
    m_dlWaitTime = response.section("var wait=", 1, 1).section(';', 0, 0).toInt();

    if ((m_captchaId.isEmpty()) || (m_captchaUrl.isEmpty()) || (m_dlWaitTime <= 0)) {
        emit error(UnknownError);
    }
    else {
        emit statusChanged(CaptchaRequired);
    }

    reply->deleteLater();
}

void ShareOnline::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("dl_free=1&captcha=%1&recaptcha_challenge_field=%2&recaptcha_response_field=%3").arg(m_captchaId).arg(challenge).arg(response);
    QNetworkRequest request(m_captchaUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Requested-With", "XMLHttpRequest");
    request.setRawHeader("Referer", "http://www.share-online.biz/dl/" + m_fileId.toUtf8() + "/free/");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ShareOnline::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://dlw\\d+-\\d.share-online.biz/fl\\?fr=[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else {
        QString response(reply->readAll());

        if (response == "0") {
            emit error(CaptchaError);
        }
        else {
            response = QByteArray::fromBase64(response.toUtf8());
            
            if (re.indexIn(response) >= 0) {
                m_downloadUrl.setUrl(response);
                this->startWait(m_dlWaitTime * 1000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
            }
            else {
                emit error(UnknownError);
            }
        }
    }

    reply->deleteLater();
}

void ShareOnline::startWait(int msecs) {
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

void ShareOnline::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void ShareOnline::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool ShareOnline::cancelCurrentOperation() {
    m_waitTimer->stop();
    m_downloadUrl.clear();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(shareonline, ShareOnline)
