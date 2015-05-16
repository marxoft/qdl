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

#include "asfile.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

using namespace QtJson;

ASfile::ASfile(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)

{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp ASfile::urlPattern() const {
    return QRegExp("http(s|)://(www.|)asfile.com/file/\\w+", Qt::CaseInsensitive);
}

bool ASfile::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void ASfile::login(const QString &username, const QString &password) {
    QUrl url("http://asfile.com/en/login");
    QString data = QString("login=%1&password=%2").arg(username).arg(password);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ASfile::checkLogin() {
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

void ASfile::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ASfile::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+\\d+\\.asfile.com/file/\\w+/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("File does not exist")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("name: '", 1, 1).section('\'', 0, 0).trimmed();

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

void ASfile::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ASfile::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+\\d+\\.asfile.com/file/\\w+/[^'\"]+");
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
        else if (!redirect.isEmpty()) {
            this->getDownloadRequest(QUrl(redirect));
        }
        else if (response.contains("File does not exist")) {
            emit error(NotFound);
        }
        else if (response.contains("Please try again later")) {
            this->startWait(600000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            m_captchaKey = response.section("google.com/recaptcha/api/challenge?k=", 1, 1).section('"', 0, 0);
            m_fileId = response.section("path: '", 1, 1).section('\'', 0, 0);
            m_fileName = response.section("name: '", 1, 1).section('\'', 0, 0);
            m_hash = response.section("hash: '", 1, 1).section('\'', 0, 0);
            m_storage = response.section("storage: '", 1, 1).section('\'', 0, 0);

            if ((m_captchaKey.isEmpty()) || (m_fileId.isEmpty()) || (m_fileName.isEmpty()) || (m_hash.isEmpty()) || (m_storage.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                int secs = response.section("var timer =", 1, 1).section(';', 0, 0).trimmed().toInt();

                if (secs > 0) {
                    this->startWait(secs * 1000);
                    this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
                }
                else {
                    emit error(UnknownError);
                }
            }
        }
    }

    reply->deleteLater();
}

void ASfile::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void ASfile::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("is_captch=1&recaptcha_challenge_field=%1&recaptcha_response_field=%2").arg(challenge).arg(response);
    QUrl url("http://asfile.com/file/" + m_fileId);
    QNetworkRequest request(url);
    request.setRawHeader("Referer", "http://asfile.com/file/" + m_fileId.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ASfile::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+\\d+\\.asfile.com/file/\\w+/[^'\"]+");
    QString response(reply->readAll());

    if (re.indexIn(response) >= 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else {
        int secs = response.section("var timer =", 1, 1).section(';', 0, 0).trimmed().toInt();

        if (secs > 0) {
            this->startWait(secs * 1000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadPage()));
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void ASfile::getDownloadPage() {
    QUrl url("http://asfile.com/en/free-download/file/" + m_fileId);
    QNetworkRequest request(url);
    request.setRawHeader("Referer", "http://asfile.com/file/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadPage()));
}

void ASfile::checkDownloadPage() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+\\d+\\.asfile.com/file/\\w+/[^'\"]+");
    QString response(reply->readAll());

    if (re.indexIn(response) >= 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else {
        int secs = response.section("var timer =", 1, 1).section(';', 0, 0).trimmed().toInt();

        if (secs > 0) {
            this->startWait(secs * 1000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(convertHashToLink()));
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void ASfile::convertHashToLink() {
    QString data = QString("hash=%1&path=%2&storage=%3&name=%4").arg(m_hash).arg(m_fileId).arg(m_storage).arg(m_fileName);
    QUrl url("http://asfile.com/en/index/convertHashToLink");
    QNetworkRequest request(url);
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("Referer", "http://asfile.com/en/free-download/file/" + m_fileId.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(convertHashToLink()));
}

void ASfile::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap map = Json::parse(response).toMap();
    QUrl url = map.value("url").toUrl();

    if (!url.isEmpty()) {
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        emit error(UrlError);
    }

    reply->deleteLater();
}

void ASfile::startWait(int msecs) {
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

void ASfile::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void ASfile::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool ASfile::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(asfile, ASfile)
#endif
