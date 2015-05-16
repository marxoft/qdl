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

#include "googledrive.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QRegExp>
#include <QSettings>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

using namespace QtJson;

GoogleDrive::GoogleDrive(QObject *parent) :
    ServicePlugin(parent)
{
    m_formatList << 37 << 22 << 35 << 34 << 18;
}

QRegExp GoogleDrive::urlPattern() const {
    return QRegExp("http(s|)://(drive|docs).google.com/file/d/\\w+", Qt::CaseInsensitive);
}

bool GoogleDrive::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void GoogleDrive::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void GoogleDrive::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
	QUrl url = reply->request().url();

	if (redirect.scheme().isEmpty()) {
	    redirect.setScheme(url.scheme());
	}

	if (redirect.host().isEmpty()) {
	    redirect.setHost(url.host());
	}
	    
        this->checkUrl(redirect);
    }
    else {
        QString response(reply->readAll());
        bool ok = false;
	    QString ps = response.section("\"download\":", -1).section("}", 0, 0) + "}";
        QVariantMap params = Json::parse(response.section("\"download\":", -1).section("}", 0, 0) + "}", ok).toMap();

        if ((!ok) || (params.isEmpty())) {
            emit urlChecked(false);
        }
        else {
            QString fileName = params.value("filename").toString();

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

void GoogleDrive::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWebPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void GoogleDrive::checkWebPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
	QUrl url = reply->request().url();

	if (redirect.scheme().isEmpty()) {
	    redirect.setScheme(url.scheme());
	}

	if (redirect.host().isEmpty()) {
	    redirect.setHost(url.host());
	}

        this->getDownloadRequest(redirect);
    }
    else {
        QString response(reply->readAll());

        QSettings settings("QDL", "QDL");

        if ((response.contains("url_encoded_fmt_stream_map\":")) && (settings.value("Google Drive/useYouTubeForVideos", false).toBool())) {
            // Treat as YouTube video if possible

            QString formatMap = response.section("url_encoded_fmt_stream_map\":\"", 1, 1).section("\", \"", 0, 0).trimmed().replace("\\u0026", "&").remove(QRegExp("itag=\\d+"));
            QMap<int, QUrl> urlMap = this->getYouTubeVideoUrlMap(formatMap);
            int format = settings.value("Google Drive/videoFormat", 18).toInt();
            QUrl videoUrl;
            int index = m_formatList.indexOf(format);

            while ((videoUrl.isEmpty()) && (index < m_formatList.size())) {
                videoUrl = urlMap.value(m_formatList.at(index));
                index++;
            }

            if (!videoUrl.isEmpty()) {
                emit downloadRequestReady(QNetworkRequest(videoUrl));
                reply->deleteLater();
                return;
            }
        }

        bool ok = false;
        QVariantMap params = Json::parse(response.section("\"download\":", -1).section("}", 0, 0) + "}", ok).toMap();

        if ((!ok) || (params.isEmpty())) {
            emit error(UnknownError);
        }
        else {
            QUrl url = params.value("url").toUrl();

            if (url.isEmpty()) {
                emit error(UnknownError);
            }
            else {
                this->getDownloadPage(url);
            }
        }
    }

    reply->deleteLater();
}

void GoogleDrive::getDownloadPage(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void GoogleDrive::checkDownloadPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
	QUrl url = reply->request().url();

	if (redirect.scheme().isEmpty()) {
	    redirect.setScheme(url.scheme());
	}

	if (redirect.host().isEmpty()) {
	    redirect.setHost(url.host());
	}

        this->getDownloadPage(redirect);
    }
    else {
        QString response(reply->readAll());
        QString confirm = response.section("confirm=", -1).section('&', 0, 0);

        if (confirm.isEmpty()) {
            emit error(UnknownError);
        }
        else {
            QUrl url = reply->request().url();
#if QT_VERSION >= 0x050000
            QUrlQuery query(url);
            query.addQueryItem("confirm", confirm);
            url.setQuery(query);
#else
            url.addQueryItem("confirm", confirm);
#endif
            this->getRedirect(url);
        }
    }

    reply->deleteLater();
}

void GoogleDrive::getRedirect(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkRedirect()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void GoogleDrive::checkRedirect() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toUrl();
    }

    if (!redirect.isEmpty()) {
        emit downloadRequestReady(QNetworkRequest(redirect));
    }
    else {
        emit downloadRequestReady(QNetworkRequest(reply->request().url()));
    }

    reply->deleteLater();
}

QMap<int, QUrl> GoogleDrive::getYouTubeVideoUrlMap(QString page) {
    QMap<int, QUrl> urlMap;
    QStringList parts = page.split(',', QString::SkipEmptyParts);

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

    return urlMap;
}

QString GoogleDrive::unescape(const QString &s) {
    int unescapes = 0;
    QByteArray us = s.toUtf8();

    while ((us.contains('%')) && (unescapes < 10)) {
        us = QByteArray::fromPercentEncoding(us);
        unescapes++;
    }

    return QString(us);
}

bool GoogleDrive::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(googledrive, GoogleDrive)
#endif
