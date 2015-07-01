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

#include "vidme.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

Vidme::Vidme(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp Vidme::urlPattern() const {
    return QRegExp("http(s|)://vid.me/\\w+", Qt::CaseInsensitive);
}

bool Vidme::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Vidme::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Vidme::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (!redirect.isEmpty()) {
        this->checkUrl(redirect);
    }
    else {
        QString response(reply->readAll());
        
        if (response.contains("og:video:url")) {
            QString title = response.section("og:title\" content=\"", 1, 1).section('"', 0, 0).replace("&amp;", "&");
            QString ext = response.section("og:video:type\" content=\"video/", 1, 1).section('"', 0, 0);
            
            if (title.isEmpty()) {
                title = "video";
            }
            
            if (ext.isEmpty()) {
                ext = "mp4";
            }
            
            emit urlChecked(true, reply->request().url(), this->serviceName(), QString("%1.%2").arg(title).arg(ext));
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void Vidme::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Vidme::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QUrl url = response.section("og:video:url\" content=\"", -1).section('"', 0, 0).replace("&amp;", "&");

    if (url.isEmpty()) {
        emit error(UrlError);
    }
    else {
        emit downloadRequestReady(QNetworkRequest(url));
    }

    reply->deleteLater();
}

bool Vidme::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(vidme, Vidme)
#endif
