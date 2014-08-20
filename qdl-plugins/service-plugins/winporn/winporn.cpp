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

#include "winporn.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QDomDocument>
#include <QDomElement>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

WinPorn::WinPorn(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp WinPorn::urlPattern() const {
    return QRegExp("http(s|)://(www.|)winporn.com/video/\\d+", Qt::CaseInsensitive);
}

bool WinPorn::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void WinPorn::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void WinPorn::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
        this->checkUrl(redirect);
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("<title>", 1, 1).section("- Free", 0, 0).simplified();

        if (!fileName.isEmpty()) {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName + ".flv");
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void WinPorn::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void WinPorn::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString config = response.section("s1.addVariable('config', '", 1, 1).section('\'', 0, 0);
    
    if (!config.isEmpty()) {
        QUrl url("http://www.winporn.com" + config);
#if QT_VERSION >= 0x050000
        QUrlQuery query(url);
        query.addQueryItem("pkey", "94df59f8022cd03574fa333993bb252b");
        url.setQuery(query);
#else
        url.addQueryItem("pkey", "94df59f8022cd03574fa333993bb252b");
#endif
        this->getVideoUrl(url);
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void WinPorn::getVideoUrl(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkVideoUrl()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void WinPorn::checkVideoUrl() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }
    
    QDomDocument doc;
    doc.setContent(reply->readAll());
    QUrl url(doc.firstChildElement("configuration").firstChildElement("video_file").text());
    
    if (url.isValid()) {
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        emit error(UnknownError);
    }
    
    reply->deleteLater();
}

bool WinPorn::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(winporn, WinPorn)
