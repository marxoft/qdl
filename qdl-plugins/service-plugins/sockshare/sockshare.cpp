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

#include "sockshare.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

SockShare::SockShare(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp SockShare::urlPattern() const {
    return QRegExp("http(s|)://(www.|)sockshare.com/file/\\w+", Qt::CaseInsensitive);
}

bool SockShare::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void SockShare::login(const QString &username, const QString &password) {
    QString data = QString("user=%1&pass=%2&login_submit=Login").arg(username).arg(password);
    QUrl url("http://www.sockshare.com/authenticate.php?login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SockShare::checkLogin() {
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
        emit loggedIn(true);
        break;
    default:
        emit loggedIn(false);
        break;
    }

    reply->deleteLater();
}

void SockShare::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SockShare::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("/get_file.php\\?id=\\w+&key=[^\"']+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("File not found")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("var name = \"", 1, 1).section('"', 0, 0).trimmed();

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

void SockShare::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SockShare::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("/get_file.php\\?id=\\w+&key=[^\"']+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) >= 0) {
        QUrl url(re.cap());
        url.setScheme("http");
        url.setHost("www.sockshare.com");
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else if (!redirect.isEmpty()) {
        this->getDownloadRequest(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QUrl url(re.cap());
            url.setScheme("http");
            url.setHost("www.sockshare.com");
            emit downloadRequestReady(QNetworkRequest(url));
        }
        else {
            QByteArray hash = response.section("\" name=\"hash", 0, 0).section('"', -1).toUtf8();

            if (hash.isEmpty()) {
                emit error(UnknownError);
            }
            else {
                this->getDownloadLink(reply->request().url(), hash);
            }
        }
    }

    reply->deleteLater();
}

void SockShare::getDownloadLink(const QUrl &url, const QByteArray &hash) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", url.toString().toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, "confirm=Continue+as+Free+User&hash=" + hash);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SockShare::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("/get_file.php\\?id=\\w+&key=[^\"']+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) >= 0) {
        QUrl url(re.cap());
        url.setScheme("http");
        url.setHost("www.sockshare.com");
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QUrl url(re.cap());
            url.setScheme("http");
            url.setHost("www.sockshare.com");
            emit downloadRequestReady(QNetworkRequest(url));
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

bool SockShare::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(sockshare, SockShare)
