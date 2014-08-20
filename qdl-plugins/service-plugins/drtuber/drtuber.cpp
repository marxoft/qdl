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

#include "drtuber.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QDomDocument>
#include <QDomElement>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

DrTuber::DrTuber(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp DrTuber::urlPattern() const {
    return QRegExp("http(s|)://(www.|)drtuber.com/video/\\d+", Qt::CaseInsensitive);
}

bool DrTuber::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void DrTuber::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void DrTuber::checkUrlIsValid() {
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

void DrTuber::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void DrTuber::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    
    QUrl url("http://www.drtuber.com/player/config.php");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("h", response.section("params += 'h=", 1, 1).section('\'', 0, 0));
    query.addQueryItem("t", response.section("params += '%26t=", 1, 1).section('\'', 0, 0));
    query.addQueryItem("vkey", response.section("params += '%26vkey=' + '", 1, 1).section('\'', 0, 0));
    query.addQueryItem("pkey", "d1fde79ca46cedd4306634ce82e4c69e");
    url.setQuery(query);
#else
    url.addQueryItem("h", response.section("params += 'h=", 1, 1).section('\'', 0, 0));
    url.addQueryItem("t", response.section("params += '%26t=", 1, 1).section('\'', 0, 0));
    url.addQueryItem("vkey", response.section("params += '%26vkey=' + '", 1, 1).section('\'', 0, 0));
    url.addQueryItem("pkey", "d1fde79ca46cedd4306634ce82e4c69e");
#endif
    this->getVideoUrl(url);

    reply->deleteLater();
}

void DrTuber::getVideoUrl(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkVideoUrl()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void DrTuber::checkVideoUrl() {
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

bool DrTuber::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(drtuber, DrTuber)
