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

#include "youtube.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QRegExp>
#include <QSettings>
#include <QDomDocument>
#include <QDomElement>
#include <QScriptEngine>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

QScriptEngine* decryptionEngine = 0;
QMap<QUrl, QScriptValue> decryptionCache;

using namespace QtJson;

YouTube::YouTube(QObject *parent) :
    ServicePlugin(parent)
{
    m_formatList << 37 << 22 << 35 << 34 << 18 << 36 << 17;

    if (!decryptionEngine) {
	decryptionEngine = new QScriptEngine;
    }
}

QRegExp YouTube::urlPattern() const {
    return QRegExp("(http(s|)://(www.|m.|)youtube.com/(v/|.+)(v=|list=|)|http://youtu.be/)", Qt::CaseInsensitive);
}

bool YouTube::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void YouTube::checkUrl(const QUrl &webUrl) {
    QString urlString = webUrl.toString();
    QString id(urlString.section(QRegExp("v=|list=|/"), -1).section(QRegExp("&|\\?"), 0, 0));
    QUrl url;
#if QT_VERSION >= 0x050000
    QUrlQuery query;

    if (urlString.contains("list=")) {
        // QUrl::hasQueryItem() does not work :/
        url.setUrl("https://gdata.youtube.com/feeds/api/playlists/" + id);
        query.addQueryItem("fields", "openSearch:totalResults,openSearch:startIndex,entry(content,media:group(media:title))");
        query.addQueryItem("max-results", "50");
    }
    else {
        url.setUrl("https://gdata.youtube.com/feeds/api/videos/" + id);
        query.addQueryItem("fields", "content,media:group(media:title)");
    }

    query.addQueryItem("v", "2.1");
    url.setQuery(query);
#else
    if (urlString.contains("list=")) {
        // QUrl::hasQueryItem() does not work :/
        url.setUrl("https://gdata.youtube.com/feeds/api/playlists/" + id);
        url.addQueryItem("fields", "openSearch:totalResults,openSearch:startIndex,entry(content,media:group(media:title))");
        url.addQueryItem("max-results", "50");
    }
    else {
        url.setUrl("https://gdata.youtube.com/feeds/api/videos/" + id);
        url.addQueryItem("fields", "content,media:group(media:title)");
    }

    url.addQueryItem("v", "2.1");
#endif
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void YouTube::checkPlaylistVideoUrls(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void YouTube::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QDomDocument doc;
    doc.setContent(reply->readAll());
    QDomNodeList entries = doc.elementsByTagName("entry");

    if (entries.isEmpty()) {
        emit urlChecked(false);
    }
    else {
        for (int i = 0; i < entries.count(); i++) {
            QDomNode entry = entries.at(i);
            QUrl url(entry.firstChildElement("content").attribute("src"));
            QString title = entry.firstChildElement("media:group").firstChildElement("media:title").text().trimmed();
            emit urlChecked((url.isValid()) && (!title.isEmpty()), url, this->serviceName(), title + ".mp4", i == (entries.count() - 1));
        }

        QDomElement resultsElement = doc.namedItem("feed").firstChildElement("openSearch:totalResults");
        QDomElement startElement = doc.namedItem("feed").firstChildElement("openSearch:startIndex");

        if ((!resultsElement.isNull()) && (!startElement.isNull())) {
            int totalResults = resultsElement.text().toInt();
            int startIndex = startElement.text().toInt();

            if (totalResults > (startIndex + entries.count())) {
                QString urlString = reply->request().url().toString();
                QUrl playlistUrl(urlString.section("&start-index=", 0, 0));
#if QT_VERSION >= 0x050000
                QUrlQuery query(playlistUrl);
                query.addQueryItem("start-index", QString::number(startIndex + entries.count()));
                playlistUrl.setQuery(query);
#else
                playlistUrl.addQueryItem("start-index", QString::number(startIndex + entries.count()));
#endif
                this->checkPlaylistVideoUrls(playlistUrl);
            }
        }
    }

    reply->deleteLater();
}

void YouTube::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QString id(webUrl.toString().section(QRegExp("v=|/"), -1).section(QRegExp("&|\\?"), 0, 0));
    this->getYouTubeVideoInfoPage(id);
}

void YouTube::getYouTubeVideoInfoPage(const QString &id) {
    QUrl url("https://www.youtube.com/get_video_info");
#if QT_VERSION >= 0x050000
    QUrlQuery query;
    query.addQueryItem("video_id", id);
    query.addQueryItem("el", "detailpage");
    query.addQueryItem("ps", "default");
    query.addQueryItem("eurl", "gl");
    query.addQueryItem("gl", "US");
    query.addQueryItem("hl", "en");
    url.setQuery(query);
#else
    url.addQueryItem("video_id", id);
    url.addQueryItem("el", "detailpage");
    url.addQueryItem("ps", "default");
    url.addQueryItem("eurl", "gl");
    url.addQueryItem("gl", "US");
    url.addQueryItem("hl", "en");
#endif
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkYouTubeVideoInfoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void YouTube::checkYouTubeVideoInfoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    if (!response.contains("url_encoded_fmt_stream_map=")) {
#if QT_VERSION >= 0x050000
        this->getYouTubeVideoWebPage(QUrlQuery(reply->request().url()).queryItemValue("video_id"));
#else
        this->getYouTubeVideoWebPage(reply->request().url().queryItemValue("video_id"));
#endif
    }
    else {
        response = response.section("url_encoded_fmt_stream_map=", 1, 1);
        QString separator = response.left(response.indexOf('%'));

        if ((separator == "s") || (response.contains("%26s%3D"))) {
#if QT_VERSION >= 0x050000
            this->getYouTubeVideoWebPage(QUrlQuery(reply->request().url()).queryItemValue("video_id"));
#else
            this->getYouTubeVideoWebPage(reply->request().url().queryItemValue("video_id"));
#endif
        }
        else {
            response = response.section("&", 0, 0).replace("%2C", ",");
            this->parseYouTubeVideoPage(QScriptValue(), response);
        }
    }

    reply->deleteLater();
}

void YouTube::getYouTubeVideoWebPage(const QString &id) {
    QUrl url("https://www.youtube.com/watch");
#if QT_VERSION >= 0x050000
    QUrlQuery query;
    query.addQueryItem("v", id);
    query.addQueryItem("gl", "US");
    query.addQueryItem("hl", "en");
    query.addQueryItem("has_verified", "1");
    url.setQuery(query);
#else
    url.addQueryItem("v", id);
    url.addQueryItem("gl", "US");
    url.addQueryItem("hl", "en");
    url.addQueryItem("has_verified", "1");
#endif
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkYouTubeWebPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void YouTube::checkYouTubeWebPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    if (!response.contains("url_encoded_fmt_stream_map\":")) {
        emit error(UnknownError);
    }
    else {
        QVariantMap assets = Json::parse(QString("%1}").arg(response.section("\"assets\": ", 1, 1).section('}', 0, 0))).toMap();
        QUrl playerUrl = assets.value("js").toUrl();

        if (playerUrl.scheme().isEmpty()) {
            playerUrl.setScheme("http");
        }

        response = response.section("url_encoded_fmt_stream_map\": \"", 1, 1).section(", \"", 0, 0).trimmed().replace("\\u0026", "&").remove(QRegExp("itag=\\d+"));

        bool encryptedSignatures = !response.contains("sig=");

        if (encryptedSignatures) {
            if (playerUrl.isValid()) {
                QScriptValue decryptionFunction = this->getYouTubeDecryptionFunction(playerUrl);

                if (decryptionFunction.isFunction()) {
                    this->parseYouTubeVideoPage(decryptionFunction, response);
                }
                else {
                    m_youtubePage = response;
                    this->connect(this, SIGNAL(youtubeDecryptionFunctionReady(QScriptValue)), this, SLOT(parseYouTubeVideoPage(QScriptValue)));
                }
            }
            else {
                emit error(UnknownError);
            }
        }
        else {
            this->parseYouTubeVideoPage(QScriptValue(), response);
        }
    }

    reply->deleteLater();
}

void YouTube::parseYouTubeVideoPage(QScriptValue decryptionFunction, QString page) {
    this->disconnect(this, SIGNAL(youtubeDecryptionFunctionReady(QScriptValue)), this, SLOT(parseYouTubeVideoPage(QScriptValue)));

    if (page.isEmpty()) {
        page = m_youtubePage;
    }

    QMap<int, QUrl> urlMap = this->getYouTubeVideoUrlMap(page, decryptionFunction);
    int format = QSettings("QDL", "QDL").value("YouTube/videoFormat", 18).toInt();
    QUrl videoUrl;
    int index = m_formatList.indexOf(format);

    while ((videoUrl.isEmpty()) && (index < m_formatList.size())) {
        videoUrl = urlMap.value(m_formatList.at(index));
        index++;
    }

    if (videoUrl.isEmpty()) {
        emit error(UnknownError);
    }
    else {
        emit downloadRequestReady(QNetworkRequest(videoUrl));
    }
}

QScriptValue YouTube::getYouTubeDecryptionFunction(const QUrl &playerUrl) {
    if (decryptionCache.contains(playerUrl)) {
        return decryptionCache.value(playerUrl);
    }

    QNetworkRequest request(playerUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(addYouTubeDecryptionFunctionToCache()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));

    return QScriptValue();
}

void YouTube::addYouTubeDecryptionFunctionToCache() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        this->disconnect(this, SIGNAL(youtubeDecryptionFunctionReady(QScriptValue)), 0, 0);
        return;
    }

    QString response(reply->readAll());
    QString funcName = response.section("signature=", 1, 1).section('(', 0, 0);
    QString var = response.section("function " + funcName, 0, 0).section(";var", -1);
    QString funcBody = QString("function %2%3").arg(funcName).arg(response.section("function " + funcName, 1, 1).section(";function", 0, 0));
    QString js = QString("var%1 %2").arg(var).arg(funcBody);
    decryptionEngine->evaluate(js);

    QScriptValue global = decryptionEngine->globalObject();
    QScriptValue decryptionFunction = global.property(funcName);

    if (decryptionFunction.isFunction()) {
        decryptionCache[reply->request().url()] = decryptionFunction;
        emit youtubeDecryptionFunctionReady(decryptionFunction);
    }
    else {
        emit error(UnknownError);
        this->disconnect(this, SIGNAL(youtubeDecryptionFunctionReady(QScriptValue)), 0, 0);
    }

    reply->deleteLater();
}

QMap<int, QUrl> YouTube::getYouTubeVideoUrlMap(QString page, QScriptValue decryptionFunction) {
    QMap<int, QUrl> urlMap;
    QStringList parts = page.split(',', QString::SkipEmptyParts);

    if (decryptionFunction.isFunction()) {
        foreach (QString part, parts) {
            part = this->unescape(part);
            part.replace(QRegExp("(^|&)s="), "&signature=");
            QString oldSig = part.section("signature=", 1, 1).section('&', 0, 0);
            part.replace(oldSig, decryptionFunction.call(QScriptValue(), QScriptValueList() << oldSig).toString());
            QStringList splitPart = part.split("url=");

            if (!splitPart.isEmpty()) {
                QString urlString = splitPart.last();
                QStringList params = urlString.mid(urlString.indexOf('?') + 1).split('&', QString::SkipEmptyParts);
                params.removeDuplicates();

                QUrl url(urlString.left(urlString.indexOf('?')));
#if QT_VERSION >= 0x050000
                QUrlQuery query;

                foreach (QString param, params) {
                    query.addQueryItem(param.section('=', 0, 0), param.section('=', -1));
                }

                if (!query.hasQueryItem("signature")) {
                    query.addQueryItem("signature", splitPart.first().section("signature=", 1, 1).section('&', 0, 0));
                }

                url.setQuery(query);

                urlMap[query.queryItemValue("itag").toInt()] = url;
#else
                foreach (QString param, params) {
                    url.addQueryItem(param.section('=', 0, 0), param.section('=', -1));
                }

                if (!url.hasQueryItem("signature")) {
                    url.addQueryItem("signature", splitPart.first().section("signature=", 1, 1).section('&', 0, 0));
                }

                urlMap[url.queryItemValue("itag").toInt()] = url;
#endif
            }
        }
    }
    else {
        foreach (QString part, parts) {
            part = this->unescape(part);
            part.replace(QRegExp("(^|&)sig="), "&signature=");
            QStringList splitPart = part.split("url=");

            if (!splitPart.isEmpty()) {
                QString urlString = splitPart.last();
                QStringList params = urlString.mid(urlString.indexOf('?') + 1).split('&', QString::SkipEmptyParts);
                params.removeDuplicates();

                QUrl url(urlString.left(urlString.indexOf('?')));
#if QT_VERSION >= 0x050000
                QUrlQuery query;

                foreach (QString param, params) {
                    query.addQueryItem(param.section('=', 0, 0), param.section('=', -1));
                }

                if (!query.hasQueryItem("signature")) {
                    query.addQueryItem("signature", splitPart.first().section("signature=", 1, 1).section('&', 0, 0));
                }

                url.setQuery(query);

                urlMap[query.queryItemValue("itag").toInt()] = url;
#else
                foreach (QString param, params) {
                    url.addQueryItem(param.section('=', 0, 0), param.section('=', -1));
                }

                if (!url.hasQueryItem("signature")) {
                    url.addQueryItem("signature", splitPart.first().section("signature=", 1, 1).section('&', 0, 0));
                }

                urlMap[url.queryItemValue("itag").toInt()] = url;
#endif
            }
        }
    }

    return urlMap;
}

QString YouTube::unescape(const QString &s) {
    int unescapes = 0;
    QByteArray us = s.toUtf8();

    while ((us.contains('%')) && (unescapes < 10)) {
        us = QByteArray::fromPercentEncoding(us);
        unescapes++;
    }

    return QString(us);
}

bool YouTube::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(youtube, YouTube)
