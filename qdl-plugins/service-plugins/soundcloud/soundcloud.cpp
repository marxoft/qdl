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

#include "soundcloud.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#define CLIENT_ID "176d25110130f29509dc252c529fbd61"
#define IPHONE_CLIENT_ID "376f225bf427445fc4bfb6b99b72e0bf"

using namespace QtJson;

SoundCloud::SoundCloud(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp SoundCloud::urlPattern() const {
    return QRegExp("http(s|)://(www.|)soundcloud.com/\\w+", Qt::CaseInsensitive);
}

bool SoundCloud::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void SoundCloud::checkUrl(const QUrl &webUrl) {
    QUrl url;

    if (webUrl.hasQueryItem("client_id")) {
        url = webUrl;
        url.setHost("api.soundcloud.com");
    }
    else {
        url.setUrl("http://api.soundcloud.com/resolve.json");
#if QT_VERSION >= 0x050000
        QUrlQuery query(url);
        query.addQueryItem("url", webUrl.toString());
        query.addQueryItem("client_id", CLIENT_ID);
        url.setQuery(query);
#else
        url.addQueryItem("url", webUrl.toString());
        url.addQueryItem("client_id", CLIENT_ID);
#endif
    }

    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SoundCloud::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

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
        QVariantMap result = Json::parse(response).toMap();

        if (!result.isEmpty()) {
            QVariantList tracks = result.value("tracks").toList();

            if (!tracks.isEmpty()) {
                while (!tracks.isEmpty()) {
                    QVariantMap track = tracks.takeFirst().toMap();
                    QUrl url(track.value("permalink_url").toString());
                    QString title = track.value("title").toString().trimmed();
                    emit urlChecked((url.isValid()) && (!title.isEmpty()), url, this->serviceName(), title + ".mp3", tracks.isEmpty());
                }
            }
            else {
                QUrl url(result.value("permalink_url").toString());
                QString title = result.value("title").toString().trimmed();
                emit urlChecked((url.isValid()) && (!title.isEmpty()), url, this->serviceName(), title + ".mp3");
            }
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void SoundCloud::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QUrl url;

    if (webUrl.hasQueryItem("client_id")) {
        url = webUrl;
        url.setHost("api.soundcloud.com");
    }
    else {
        url.setUrl("http://api.soundcloud.com/resolve.json");
#if QT_VERSION >= 0x050000
        QUrlQuery query(url);
        query.addQueryItem("url", webUrl.toString());
        query.addQueryItem("client_id", CLIENT_ID);
        url.setQuery(query);
#else
        url.addQueryItem("url", webUrl.toString());
        url.addQueryItem("client_id", CLIENT_ID);
#endif
    }

    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseJson()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SoundCloud::parseJson() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
        this->getDownloadRequest(redirect);
    }
    else {
        QString response(reply->readAll());
        QVariantMap track = Json::parse(response).toMap();

        if (!track.isEmpty()) {
            QString format = QSettings("QDL", "QDL").value("SoundCloud/audioFormat", "original").toString();
            QString downloadUrl = track.value("download_url").toString();
            QUrl url;

            if ((format == "original") && (!downloadUrl.isEmpty())) {
                url.setUrl(downloadUrl);
#if QT_VERSION >= 0x050000
                QUrlQuery query(url);
                query.addQueryItem("client_id", CLIENT_ID);
                url.setQuery(query);
#else
                url.addQueryItem("client_id", CLIENT_ID);
#endif
                emit downloadRequestReady(QNetworkRequest(url));
            }
            else  {
                QString id = track.value("id").toString();
                
                if (!id.isEmpty()) {
                    this->getStreamUrl(id);
                }
                else {
                    emit error(UnknownError);
                }
            }
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void SoundCloud::getStreamUrl(const QString &id, const QString &token) {
    QUrl url(QString("http://api.soundcloud.com/i1/tracks/%1/streams").arg(id));
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("client_id", IPHONE_CLIENT_ID);

    if (!token.isEmpty()) {
        query.addQueryItem("secret_token", token);
    }
#else
    url.addQueryItem("client_id", IPHONE_CLIENT_ID);
    
    if (!token.isEmpty()) {
        url.addQueryItem("secret_token", token);
    }
#endif
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(extractStreamUrl()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SoundCloud::extractStreamUrl() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QVariantMap formats = Json::parse(response).toMap();
    QMapIterator<QString, QVariant> iterator(formats);
    QUrl url;

    while ((iterator.hasNext()) && (url.isEmpty())) {
        iterator.next();
        
        if (iterator.key().startsWith("http")) {
            url.setUrl(iterator.value().toString());
        }
        else if (iterator.key().startsWith("rtmp")) {
            url.setUrl(iterator.value().section("mp3:", 1));
        }
    }

    if (url.isValid()) {
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

bool SoundCloud::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(soundcloud, SoundCloud)
#endif
