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

#include "bliptv.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>
#include <QDebug>

Bliptv::Bliptv(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp Bliptv::urlPattern() const {
    return QRegExp("http(s|)://(www.|)blip.tv/\\w+", Qt::CaseInsensitive);
}

bool Bliptv::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Bliptv::checkUrl(const QUrl &webUrl) {
    QUrl url("http://blip.tv/players/xplayer");
    url.addQueryItem("id", webUrl.toString().section('/', -1).section('-', -1));
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Bliptv::checkUrlIsValid() {
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
        QString url = response.section("data-blipsd=\"", 1, 1).section('"', 0, 0);
        
        if (!url.isEmpty()) {
            QString fileName = response.section("data-episode-title=\"", 1, 1).section('"', 0, 0) + ".mp4";
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void Bliptv::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Bliptv::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }
    
    QString response(reply->readAll());
    QString format = QSettings("QDL", "QDL").value("Blip.tv/videoFormat", "sd").toString();
    QUrl url(response.section(QString("data-blip%1=\"").arg(format), 1, 1).section('"', 0, 0));
        
    if (url.isValid()) {
        this->getRedirect(url);
    }
    else if (format != "sd") {
        url.setUrl(response.section("data-blipsd=\"", 1, 1).section('"', 0, 0));
            
        if (url.isValid()) {
            this->getRedirect(url);
        }
        else {
            emit error(UnknownError);
        }
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void Bliptv::getRedirect(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkRedirect()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Bliptv::checkRedirect() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }
    
    QUrl url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    
    qDebug() << url;
    
    qDebug() << reply->rawHeader("Location");
    
    QString response(reply->readAll());
    
    QUrl redirect(response.section("href=\"", -1).section('"', 0, 0));
    
    qDebug() << redirect;
    
    if (redirect.isValid()) {
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        emit error(UnknownError);
    }
    
    reply->deleteLater();   
}

bool Bliptv::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(bliptv, Bliptv)
#endif
