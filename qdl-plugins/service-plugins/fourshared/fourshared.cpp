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

#include "fourshared.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QTimer>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

using namespace QtJson;

FourShared::FourShared(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_wait(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp FourShared::urlPattern() const {
    return QRegExp("http(s|)://(www.|)4shared.com/\\w+/\\w+", Qt::CaseInsensitive);
}

bool FourShared::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void FourShared::login(const QString &username, const QString &password) {
    QString data = QString("login=%1&password=%2").arg(username).arg(password);
    QUrl url("https://www.4shared.com/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FourShared::checkLogin() {
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

void FourShared::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FourShared::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+.4shared.com/download/\\w+/.+\\?tsid=[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains(QRegExp("file link that you requested is not valid|enter a password to access"))) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("<meta name=\"title\" content=\"", 1, 1).section('"', 0, 0).trimmed();

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

void FourShared::getDownloadRequest(const QUrl &webUrl) {
    if (!m_downloadUrl.isEmpty()) {
        emit downloadRequestReady(QNetworkRequest(m_downloadUrl));
        m_downloadUrl.clear();
    }
    else {
        emit statusChanged(Connecting);
        QNetworkRequest request(webUrl);
        request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
        QNetworkReply *reply = this->networkAccessManager()->get(request);
        this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
        this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    }
}

void FourShared::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.4shared.com/download/\\w+/.+\\?tsid=[^'\"]+");
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
            m_downloadUrl = QUrl(re.cap());
            m_wait = response.section("secondsLeft\" value=\"", 1, 1).section('"', 0, 0).toInt();
            QString fileId = response.section("fileId\" value=\"", 1, 1).section('"', 0, 0);

            if (fileId.isEmpty()) {
                emit error(UnknownError);
            }
            else {
                this->getDownloadLimitInfo(fileId);
            }
        }
        else {
            QString urlString = reply->request().url().toString();

            if (!urlString.contains("4shared.com/get/")) {
                this->getDownloadRequest(QUrl(urlString.replace(QRegExp("4shared.com/\\w+/"), "4shared.com/get/")));
            }
            else {
                emit error(UnknownError);
            }
        }
    }

    reply->deleteLater();
}

void FourShared::getDownloadLimitInfo(const QString &fileId) {
    QUrl url("http://www.4shared.com/web/d2/getFreeDownloadLimitInfo?fileId=" + fileId);
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLimitInfo()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FourShared::checkDownloadLimitInfo() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap map = Json::parse(response).toMap();

    if (map.value("status") == "ok") {
        this->startWait(m_wait * 1000);

        if (m_wait > 30) {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
        }
    }
    else {
        QVariantMap traffic = map.value("traffic").toMap();

        if (traffic.value("exceeded") != "none") {
            emit error(TrafficExceeded);
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void FourShared::startWait(int msecs) {
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

void FourShared::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void FourShared::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

void FourShared::getDownloadLink() {
    QNetworkRequest request(m_downloadUrl);
    QNetworkReply *reply = this->networkAccessManager()->head(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
}

void FourShared::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
#if QT_VERSION >= 0x050000
    QUrlQuery query(redirect);
    
    if (query.hasQueryItem("cau2")) {
        emit error(NotFound);
    }
#else
    if (redirect.hasQueryItem("cau2")) {
        emit error(NotFound);
    }
#endif
    else {
        emit downloadRequestReady(QNetworkRequest(m_downloadUrl));
    }

    reply->deleteLater();
}

bool FourShared::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(fourshared, FourShared)
#endif
