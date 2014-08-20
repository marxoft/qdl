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

#include "vube.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>

using namespace QtJson;

Vube::Vube(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp Vube::urlPattern() const {
    return QRegExp("http(s|)://(www.|)vube.com/\\w+", Qt::CaseInsensitive);
}

bool Vube::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Vube::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Vube::checkUrlIsValid() {
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
        QString videoDataString = response.section("[\"vubeOriginalVideoData\"] =", 1, 1).section(';', 0, 0).trimmed();
        QVariantMap videoData = Json::parse(videoDataString).toMap();

        if (!videoData.isEmpty()) {
            QString fileName = videoData.value("title").toString() + ".mp4";

            if (!fileName.isEmpty()) {
                emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
            }
            else {
                emit urlChecked(false);
            }
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void Vube::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Vube::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString videoDataString = response.section("[\"vubeOriginalVideoData\"] =", 1, 1).section(';', 0, 0).trimmed();
    QVariantMap videoData = Json::parse(videoDataString).toMap();

    if (!videoData.isEmpty()) {
        QString publicId = videoData.value("public_id").toString();
        QVariantList formats = videoData.value("mtm").toList();
        QMap<int, QUrl> urlMap;

        foreach (QVariant format, formats) {
            QVariantMap formatMap = format.toMap();
            int resId = formatMap.value("media_resolution_id").toInt();
            urlMap.insert(resId, QUrl(QString("http://video.thestaticvube.com/video/%1/%2.mp4").arg(resId).arg(publicId)));
        }

        QList<int> keys = urlMap.keys();
        int preferredFormat = QSettings("QDL", "QDL").value("Vube/videoFormat", 4).toInt();
        int i = 0;
        QUrl url;

        while ((url.isEmpty()) && (i < keys.size())) {
            if (keys.at(i) >= preferredFormat) {
                url = urlMap.value(keys.at(i));
            }

            i++;
        }

        if (!url.isEmpty()) {
            emit downloadRequestReady(QNetworkRequest(url));
        }
        else {
            emit error(UrlError);
        }
    }
    else {
        emit error(UrlError);
    }

    reply->deleteLater();
}

bool Vube::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(vube, Vube)
