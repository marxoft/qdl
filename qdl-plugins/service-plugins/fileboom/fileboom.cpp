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

#include "fileboom.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>
#include <QTime>

FileBoom::FileBoom(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp FileBoom::urlPattern() const {
    return QRegExp("http(s|)://(www.|)f(ile|)boom.me/file/\\w+", Qt::CaseInsensitive);
}

bool FileBoom::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void FileBoom::login(const QString &username, const QString &password) {
    QString data = QString("LoginForm[username]=%1&LoginForm[password]=%2").arg(username).arg(password);
    QUrl url("http://fboom.net/login.html");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileBoom::checkLogin() {
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

void FileBoom::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileBoom::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("(http://fboom.me|)/file/url.html\\?file=[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("icon-download\"></i>", 1, 1).section("</div>", 0, 0).trimmed();

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void FileBoom::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileBoom::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("(http://fboom.me|)/file/url.html\\?file=[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QUrl url(re.cap());

        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("fboom.me");
        }

        emit downloadRequestReady(QNetworkRequest(url));
    }
    else if (!redirect.isEmpty()) {
        QUrl url(redirect);

        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("fboom.me");
        }

        this->getDownloadRequest(url);
    }
    else {
        m_url = reply->request().url();

        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QUrl url(re.cap());

            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }

            emit downloadRequestReady(QNetworkRequest(url));
        }
        else {
            m_fileId = response.section("data-slow-id=\"", 1, 1).section('"', 0, 0);

            if (m_fileId.isEmpty()) {
                if (response.contains("system is unable to find")) {
                    emit error(NotFound);
                }
                else {
                    emit error(UnknownError);
                }
            }
            else {
                this->getWaitTime();
            }
        }
    }

    reply->deleteLater();
}

void FileBoom::getWaitTime() {
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, "slow_id=" + m_fileId.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileBoom::checkWaitTime() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("(http://fboom.me|)/file/url.html\\?file=[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QUrl url(re.cap());

        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("fboom.me");
        }

        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QUrl url(re.cap());

            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }

            emit downloadRequestReady(QNetworkRequest(url));
        }
        else if (response.contains("Downloading is not possible")) {
            QTime time = QTime::fromString(response.section("Please wait", 1, 1).section("to download", 0, 0).trimmed(), "hh:mm:ss");

            if (time.isValid()) {
                this->startWait(QTime::currentTime().msecsTo(time));
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
            }
            else {
                emit error(UnknownError);
            }
        }
        else {
            m_captchaKey = response.section("/file/captcha.html?v=", 1, 1).section('"', 0, 0);

            if (m_captchaKey.isEmpty()) {
                emit error(UnknownError);
            }
            else {
                emit statusChanged(CaptchaRequired);
            }
        }
    }

    reply->deleteLater();
}

void FileBoom::submitCaptchaResponse(const QString &challenge, const QString &response) {
    Q_UNUSED(challenge);

    QString data = QString("CaptchaForm[code]=%1&free=1&freeDownloadRequest=1&uniqueId=%2").arg(response).arg(m_fileId);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileBoom::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("(http://fboom.me|)/file/url.html\\?file=[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QUrl url(re.cap());

        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("fboom.me");
        }

        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QUrl url(re.cap());

            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }

            emit downloadRequestReady(QNetworkRequest(url));
        }
        else if (response.contains("verification code is incorrect")) {
            m_captchaKey = response.section("/file/captcha.html?v=", 1, 1).section('"', 0, 0);

            if (m_captchaKey.isEmpty()) {
                emit error(UnknownError);
            }
            else {
                emit statusChanged(CaptchaRequired);
            }
        }
        else {
            int secs = response.section("tik-tak\">", 1, 1).section('<', 0, 0).toInt();

            if (secs > 0) {
                this->startWait(secs * 1000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
            }
        }
    }

    reply->deleteLater();
}

void FileBoom::getDownloadLink() {
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, "free=1&uniqueId=" + m_fileId.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
}

void FileBoom::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("(http://fboom.me|)/file/url.html\\?file=[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QUrl url(re.cap());

        if (url.host().isEmpty()) {
            url.setScheme("http");
            url.setHost("fboom.me");
        }

        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QUrl url(re.cap());

            if (url.host().isEmpty()) {
                url.setScheme("http");
                url.setHost("fboom.me");
            }

            emit downloadRequestReady(QNetworkRequest(url));
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void FileBoom::startWait(int msecs) {
    if (msecs > 30000) {
        emit statusChanged(LongWait);
    }
    else {
        emit statusChanged(ShortWait);
    }

    emit waiting(msecs);
    m_waitTime = msecs;
    m_waitTimer->start(1000);
}

void FileBoom::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void FileBoom::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool FileBoom::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(fileboom, FileBoom)
#endif
