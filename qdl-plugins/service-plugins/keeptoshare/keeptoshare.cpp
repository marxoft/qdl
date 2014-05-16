#include "keeptoshare.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QTime>
#include <QRegExp>

using namespace QtJson;

KeepToShare::KeepToShare(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp KeepToShare::urlPattern() const {
    return QRegExp("http(s|)://(www.|)(keep2share|k2s).cc/file/\\w+", Qt::CaseInsensitive);
}

bool KeepToShare::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void KeepToShare::login(const QString &username, const QString &password) {
    m_user = username;
    m_pass = password;
    QString data = QString("LoginForm[username]=%1&LoginForm[password]=%2&LoginForm[rememberMe]=1").arg(username).arg(password);
    QUrl url("http://keep2share.cc/login.html");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkKeep2ShareLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void KeepToShare::checkKeep2ShareLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (statusCode) {
    case 200:
    case 201:
        this->loginK2C(m_user, m_pass);
        break;
    default:
        m_connections = 1;
        emit loggedIn(false);
        break;
    }

    m_user = "";
    m_pass = "";

    reply->deleteLater();
}

void KeepToShare::loginK2C(const QString &username, const QString &password) {
    QString data = QString("LoginForm[username]=%1&LoginForm[password]=%2&LoginForm[rememberMe]=1").arg(username).arg(password);
    QUrl url("http://k2s.cc/login.html");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkK2CLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void KeepToShare::checkK2CLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (statusCode) {
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

void KeepToShare::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void KeepToShare::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http(s|)://keep2share.cc/file/url.html[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("File: <span>", 1, 1).section('<', 0, 0);
        
        if (fileName.isEmpty()) {
            fileName = response.section("font-size: 18px\">", 1, 1).section('<', 0, 0);

            if (fileName.isEmpty()) {
                emit urlChecked(false);
            }
            else {
                emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
            }
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void KeepToShare::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void KeepToShare::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http(s|)://keep2share.cc/file/url.html[^'\"]+");
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

        if (response.contains("/file/url.html?file=")) {
	    QUrl url("http://keep2share.cc/file/url.html");
            url.addQueryItem("file", response.section("/file/url.html?file=", 1, 1).section('\'', 0, 0));
            emit downloadRequestReady(QNetworkRequest(url));
        }
        else {
            m_url = reply->request().url();
            m_fileId = response.section("slow_id\" value=\"", 1, 1).section('"', 0, 0);

            if (m_fileId.isEmpty()) {
		if (response.contains("This file is available<br>only for premium members")) {
		    emit error(Unauthorised);
		}
		else {
                    emit error(UnknownError);
		}
            }
            else {
                this->getCaptchaKey();
            }
        }
    }

    reply->deleteLater();
}

void KeepToShare::getCaptchaKey() {
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, "yt0=&slow_id=" + m_fileId.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaKey()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void KeepToShare::checkCaptchaKey() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }
    
    QString response(reply->readAll());
    QRegExp re("/file/url.html[^'\"]+");

    if (re.indexIn(response) >= 0) {
        QUrl url("http://keep2share.cc" + re.cap());
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        m_captchaKey = response.section("http://www.google.com/recaptcha/api/challenge?k=", 1, 1).section('"', 0, 0);
        
        if (m_captchaKey.isEmpty()) {
            QString waitString = response.section("Please wait", 1, 1).section("to download this file", 0, 0).trimmed();
            
            if (waitString.isEmpty()) {
                emit error(UnknownError);
            }
            else {
                int waitTime = QTime().msecsTo(QTime::fromString(waitString));
                
                if (waitTime <= 0) {
                    emit error(UnknownError);
                }
                else {
                    this->startWait(waitTime);
                    this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
                }
            }
        }
        else {
            emit statusChanged(CaptchaRequired);
        }
    }
    
    reply->deleteLater();
}

void KeepToShare::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("CaptchaForm[code]=&recaptcha_challenge_field=%1&recaptcha_response_field=%2&free=1&freeDownloadRequest=1&uniqueId=%3&yt0=").arg(challenge).arg(response).arg(m_fileId);

    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void KeepToShare::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    int downloadWaitTime = response.section("download-wait-timer\">", 1, 1).section('<', 0, 0).trimmed().toInt();
    
    if (downloadWaitTime > 0) {
        this->startWait(downloadWaitTime * 1000);
        this->connect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadRequest()));
    }
    else if (response.contains("The verification code is incorrect")) {
        emit error(CaptchaError);
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void KeepToShare::getDownloadRequest() {
    QString data = QString("uniqueId=%1&free=1").arg(m_fileId);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadRequest()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadRequest()));
}

void KeepToShare::checkDownloadRequest() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    QVariantMap result = Json::parse(response).toMap();
    QUrl url = result.value("url").toUrl();

    if (!url.isEmpty()) {
	emit downloadRequestReady(QNetworkRequest(url));
    }
    else if (response.contains("/file/url.html?file=")) {
        QUrl url("http://keep2share.cc/file/url.html");
        url.addQueryItem("file", m_fileId);
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        emit error(UnknownError);
    }
    
    reply->deleteLater();
}   

void KeepToShare::startWait(int msecs) {
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

void KeepToShare::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void KeepToShare::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool KeepToShare::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(keeptoshare, KeepToShare)
