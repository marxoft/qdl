/*
 * Copyright (C) 2014 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "keeptoshare.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QTime>
#include <QRegExp>

KeepToShare::KeepToShare(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp KeepToShare::urlPattern() const {
    return QRegExp("http(s|)://(www.|)(keep2s(hare|)|k2s).cc/file/\\w+", Qt::CaseInsensitive);
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
        this->loginK2S(m_user, m_pass);
        break;
    default:
        m_connections = 1;
        emit loggedIn(false);
        break;
    }

    reply->deleteLater();
}

void KeepToShare::loginK2S(const QString &username, const QString &password) {
    QString data = QString("LoginForm[username]=%1&LoginForm[password]=%2&LoginForm[rememberMe]=1").arg(username).arg(password);
    QUrl url("http://k2s.cc/login.html");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkK2SLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void KeepToShare::checkK2SLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (statusCode) {
    case 200:
    case 201:
        this->loginKeep2s(m_user, m_pass);
        break;
    default:
        m_connections = 1;
        emit loggedIn(false);
        break;
    }

    reply->deleteLater();
}

void KeepToShare::loginKeep2s(const QString &username, const QString &password) {
    QString data = QString("LoginForm[username]=%1&LoginForm[password]=%2&LoginForm[rememberMe]=1").arg(username).arg(password);
    QUrl url("http://keep2s.cc/login.html");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkKeep2sLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void KeepToShare::checkKeep2sLogin() {
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

    m_user = "";
    m_pass = "";

    reply->deleteLater();
}

void KeepToShare::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
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
    QRegExp re("(http://(keep2s(hare|)|k2s).cc|)/file/url.html[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("File: <span>", 1, 1).section('<', 0, 0);
        
        if (fileName.isEmpty()) {
            fileName = response.section("font-size: 18px; \">", 1, 1).section('<', 0, 0);

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

void KeepToShare::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
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

    QRegExp re("(http://(keep2s(hare|)|k2s).cc|)/file/url.html[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QUrl url(re.cap());

        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("k2s.cc");
        }

        emit downloadRequestReady(QNetworkRequest(url));
    }
    else if (!redirect.isEmpty()) {
        QUrl url(redirect);

        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("k2s.cc");
        }

        this->getDownloadRequest(url);
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QUrl url(re.cap());

            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("k2s.cc");
            }

            emit downloadRequestReady(QNetworkRequest(url));
        }
        else {
            m_url = reply->url();
            m_fileId = response.section("slow_id\" value=\"", 1, 1).section('"', 0, 0);

            if (m_fileId.isEmpty()) {
                if (response.contains("only for premium members")) {
                    emit error(Unauthorised);
                }
                else if (response.contains("not found or deleted")) {
                    emit error(NotFound);
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
    QRegExp re("(http://(keep2s(hare|)|k2s).cc|)/file/url.html[^'\"]+");

    if (re.indexIn(response) >= 0) {
        QUrl url(re.cap());

        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("k2s.cc");
        }

        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        m_url = reply->url();
        m_captchaKey = response.section("/file/captcha.html?v=", 1, 1).section('"', 0, 0);
        
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
            m_captchaKey = QString("http://%1/file/captcha.html?v=%2").arg(reply->url().host()).arg(m_captchaKey);
            emit statusChanged(CaptchaRequired);
        }
    }
    
    reply->deleteLater();
}

void KeepToShare::submitCaptchaResponse(const QString &challenge, const QString &response) {
    Q_UNUSED(challenge);

    QString data = QString("CaptchaForm[code]=%1&free=1&freeDownloadRequest=1&uniqueId=%2&yt0=").arg(response).arg(m_fileId);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
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
    else if (response.contains("verification code is incorrect")) {
        m_captchaKey = response.section("/file/captcha.html?v=", 1, 1).section('"', 0, 0);

        if (m_captchaKey.isEmpty()) {
            emit error(UnknownError);
        }
        else {
            m_captchaKey = QString("http://%1/file/captcha.html?v=%2").arg(reply->url().host()).arg(m_captchaKey);
            emit error(CaptchaError);
        }
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void KeepToShare::getDownloadRequest() {
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, "free=1&uniqueId=" + m_fileId.toUtf8());
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

    QRegExp re("(http://(keep2s(hare|)|k2s).cc|)/file/url.html[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QUrl url(re.cap());

        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("k2s.cc");
        }

        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QUrl url(re.cap());

            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("k2s.cc");
            }

            emit downloadRequestReady(QNetworkRequest(url));
        }
        else {
            emit error(UnknownError);
        }
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
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(keeptoshare, KeepToShare)
