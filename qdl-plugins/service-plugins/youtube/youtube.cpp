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
#include <qyoutube/resourcesrequest.h>
#include <qyoutube/streamsrequest.h>
#include <QNetworkRequest>
#include <QStringList>
#include <QRegExp>
#include <QSettings>

static const QString API_KEY("AIzaSyDhIlkLzHJKDCNr6thsjlQpZrkY3lO_Uu4");
static const QString CLIENT_ID("957843447749-ur7hg6de229ug0svjakaiovok76s6ecr.apps.googleusercontent.com");
static const QString CLIENT_SECRET("dDs2_WwgS16LZVuzqA9rIg-I");
static const QStringList FORMAT_LIST = QStringList() << "37" << "22" << "35" << "34" << "18" << "36" << "17";

YouTube::YouTube(QObject *parent) :
    ServicePlugin(parent),
    m_resourcesRequest(0),
    m_streamsRequest(0)
{
}

QRegExp YouTube::urlPattern() const {
    return QRegExp("(http(s|)://(www.|m.|)youtube.com/(v/|.+)(v=|list=|)|http://youtu.be/)", Qt::CaseInsensitive);
}

bool YouTube::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void YouTube::checkUrl(const QUrl &url) {
    if (!m_resourcesRequest) {
        m_resourcesRequest = new QYouTube::ResourcesRequest(this);
        m_resourcesRequest->setApiKey(API_KEY);
        m_resourcesRequest->setClientId(CLIENT_ID);
        m_resourcesRequest->setClientSecret(CLIENT_SECRET);
        this->connect(m_resourcesRequest, SIGNAL(finished()), this, SLOT(onResourcesRequestFinished()));
    }
    
    QString urlString = url.toString();
    QString id(urlString.section(QRegExp("v=|list=|/"), -1).section(QRegExp("&|\\?"), 0, 0));
    
    if (urlString.contains("list=")) {
        QVariantMap filters;
        filters["playlistId"] = id;
        QVariantMap params;
        params["maxResults"] = 50;
        m_resourcesRequest->list("/playlistItems", QStringList() << "snippet", filters, params);
    }
    else {
        QVariantMap filters;
        filters["id"] = id;
        m_resourcesRequest->list("/videos", QStringList() << "snippet", filters);
    }
}

void YouTube::getDownloadRequest(const QUrl &url) {    
    if (!m_streamsRequest) {
        m_streamsRequest = new QYouTube::StreamsRequest(this);
        this->connect(m_streamsRequest, SIGNAL(finished()), this, SLOT(onStreamsRequestFinished()));
    }
    
    emit statusChanged(Connecting);
    QString id(url.toString().section(QRegExp("v=|/"), -1).section(QRegExp("&|\\?"), 0, 0));
    m_streamsRequest->list(id);
}

void YouTube::onResourcesRequestFinished() {
    if (m_resourcesRequest->status() == QYouTube::ResourcesRequest::Ready) {
        QVariantMap result = m_resourcesRequest->result().toMap();
        QVariantList list = result.value("items").toList();
        
        if (list.isEmpty()) {
            emit urlChecked(false);
            return;
        }
        
        
        QString nextPageToken = result.value("nextPageToken").toString();
                 
        while (!list.isEmpty()) {
            QVariantMap item = list.takeFirst().toMap();
            QVariantMap snippet = item.value("snippet").toMap();
            QString title = snippet.value("title").toString();
            QString id = (item.value("kind") == "youtube#playlistItem"
                          ? snippet.value("resourceId").toMap().value("videoId").toString()
                          : item.value("id").toString());
            QUrl url("https://www.youtube.com/watch?v=" + id);
            emit urlChecked(true, url, this->serviceName(), title + ".mp4",
                            (list.isEmpty()) && (nextPageToken.isEmpty()));
        }
        
        if (!nextPageToken.isEmpty()) {
            QVariantMap filters;
            filters["nextPageToken"] = nextPageToken;
            QVariantMap params;
            params["maxResults"] = 50;
            m_resourcesRequest->list("/playlistItems", QStringList() << "snippet", filters, params);
        }
    }
    else if (m_resourcesRequest->status() == QYouTube::ResourcesRequest::Failed) {
        emit urlChecked(false);
    }
}

void YouTube::onStreamsRequestFinished() {
    if (m_streamsRequest->status() == QYouTube::StreamsRequest::Ready) {
        QVariantList list = m_streamsRequest->result().toList();
        
        if (list.isEmpty()) {
            emit error(UrlError);
            return;
        }
        
        QString format = QSettings("QDL", "QDL").value("YouTube/videoFormat", "18").toString();
        
        for (int i = FORMAT_LIST.indexOf(format); i < FORMAT_LIST.size(); i++) {
            foreach (QVariant item, list) {
                QVariantMap stream = item.toMap();
                
                if (stream.value("id") == FORMAT_LIST.at(i)) {
                    emit downloadRequestReady(QNetworkRequest(stream.value("url").toString()));
                    return;
                }
            }
        }
        
        emit error(UrlError);
    }
    else if (m_streamsRequest->status() == QYouTube::StreamsRequest::Failed) {
        emit error(UrlError);
    }   
}

bool YouTube::cancelCurrentOperation() {
    if (m_resourcesRequest) {
        m_resourcesRequest->cancel();
    }
    
    if (m_streamsRequest) {
        m_streamsRequest->cancel();
    }
    
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(youtube, YouTube)
#endif
