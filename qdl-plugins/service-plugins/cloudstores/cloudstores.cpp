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

#include "cloudstores.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

CloudStores::CloudStores(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp CloudStores::urlPattern() const {
    return QRegExp("http(s|)://(www.|)cloudstor.es/f/[\\w-_]+", Qt::CaseInsensitive);
}

bool CloudStores::urlSupported(const QUrl &url) const {
    return urlPattern().indexIn(url.toString()) == 0;
}

void CloudStores::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void CloudStores::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (!redirect.isEmpty()) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("dl_mime", 1, 1).section("<h1>", 1, 1).section("<", 0, 0);

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void CloudStores::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void CloudStores::checkDownloadPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QUrl url("http://cloudstor.es" + response.section("url: '", 1, 1).section('\'', 0, 0));
    QString id = response.section("id: '", 1, 1).section('\'', 0, 0);
    QString part = response.section("part: '", 1, 1).section('\'', 0, 0);
    QString token = response.section("token: '", 1, 1).section('\'', 0, 0);

    if ((id.isEmpty()) || (part.isEmpty()) || (token.isEmpty())) {
        emit error(UnknownError);
    }
    else {
        QString data = QString("id=%1&part=%2&token=%3").arg(id).arg(part).arg(token);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        request.setRawHeader("X-Requested-With", "XMLHttpRequest");
        request.setRawHeader("Referer", reply->request().url().toString().toUtf8());
        emit downloadRequestReady(request, data.toUtf8());
    }

    reply->deleteLater();
}

bool CloudStores::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(cloudstores, CloudStores)
