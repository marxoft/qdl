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

#include "dailymotion.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

using namespace QtJson;

Dailymotion::Dailymotion(QObject *parent) :
    ServicePlugin(parent)
{
    m_formatList << "stream_h264_hd1080_url" << "stream_h264_hd_url" << "stream_h264_hq_url" << "stream_h264_url" << "stream_h264_ld_url";
}

QRegExp Dailymotion::urlPattern() const {
    return QRegExp("(http(s|)://(www.|)dailymotion.com/(video|playlist)/|http://dai.ly/)\\w{6,7}", Qt::CaseInsensitive);
}

bool Dailymotion::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Dailymotion::checkUrl(const QUrl &webUrl) {
    QString urlString = webUrl.toString();
    QRegExp re("\\w{6,7}(_|$)");
    re.indexIn(urlString);
    QString id = re.cap();
    QUrl url;
#if QT_VERSION >= 0x050000
    QUrlQuery query;

    if (urlString.contains("/playlist/")) {
        url.setUrl(QString("https://api.dailymotion.com/playlist/%1/videos").arg(id));
        query.addQueryItem("limit", "100");
    }
    else {
        url.setUrl("https://api.dailymotion.com/video/" + id);
    }

    query.addQueryItem("fields", "title,url");
    url.setQuery(query);
#else
    if (urlString.contains("/playlist/")) {
        url.setUrl(QString("https://api.dailymotion.com/playlist/%1/videos").arg(id));
        url.addQueryItem("limit", "100");
    }
    else {
        url.setUrl("https://api.dailymotion.com/video/" + id);
    }

    url.addQueryItem("fields", "title,url");
#endif
    QNetworkRequest request(url);
    request.setRawHeader("Cookie", "family_filter=false");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Dailymotion::checkPlaylistVideoUrls(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("Cookie", "family_filter=false");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Dailymotion::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString response(reply->readAll());
    QVariantMap result = Json::parse(response).toMap();
#if QT_VERSION >= 0x050000
    QUrlQuery query(reply->request().url());

    if (query.hasQueryItem("limit")) {
#else
    if (reply->request().url().hasQueryItem("limit")) {
#endif
        QVariantList videos = result.value("list").toList();
        bool moreResults = result.value("has_more").toBool();

        if (videos.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            while (!videos.isEmpty()) {
                QVariantMap video = videos.takeFirst().toMap();
                QUrl url(video.value("url").toString());
                QString title = video.value("title").toString().trimmed();
                emit urlChecked((url.isValid()) && (!title.isEmpty()), url, this->serviceName(), title + ".mp4", (videos.isEmpty()) && (!moreResults));
            }

            if (moreResults) {
                QString urlString = reply->request().url().toString();
                QUrl playlistUrl(urlString.section("&page=", 0, 0));
#if QT_VERSION >= 0x050000
                QUrlQuery query(playlistUrl);
                query.addQueryItem("page", QString::number(result.value("page").toInt() + 1));
                playlistUrl.setQuery(query);
#else
                playlistUrl.addQueryItem("page", QString::number(result.value("page").toInt() + 1));
#endif
                this->checkPlaylistVideoUrls(playlistUrl);
            }
        }
    }
    else {
        QUrl url(result.value("url").toString());
        QString title = result.value("title").toString().trimmed();
        emit urlChecked((url.isValid()) && (!title.isEmpty()), url, this->serviceName(), title + ".mp4");
    }

    reply->deleteLater();
}

void Dailymotion::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QString id = webUrl.toString().section('/', -1).section('_', 0, 0);
    QUrl url("http://www.dailymotion.com/embed/video/" + id);
    QNetworkRequest request(url);
    request.setRawHeader("Cookie", "family_filter=false");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Dailymotion::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap info = Json::parse(response.section("var info = ", 1, 1).section(";\n", 0, 0)).toMap();
    QUrl url;
    int i = m_formatList.indexOf(QSettings("QDL", "QDL").value("Dailymotion/videoFormat", "stream_h264_url").toString());

    while ((url.isEmpty()) && (i < m_formatList.size())) {
        url.setUrl(info.value(m_formatList.at(i)).toString());
        i++;
    }

    if (url.isEmpty()) {
        this->setErrorString(info.value("error").toMap().value("message").toString());
        emit error(UnknownError);
    }
    else {
        this->getVideoUrl(url);
    }

    reply->deleteLater();
}

void Dailymotion::getVideoUrl(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("Cookie", "family_filter=false");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkVideoUrl()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Dailymotion::checkVideoUrl() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (redirect.isEmpty()) {
        emit downloadRequestReady(QNetworkRequest(reply->request().url()));
    }
    else {
        emit downloadRequestReady(QNetworkRequest(redirect));
    }

    reply->deleteLater();
}

bool Dailymotion::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(dailymotion, Dailymotion)
#endif
