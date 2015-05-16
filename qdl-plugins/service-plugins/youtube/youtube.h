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

#ifndef YOUTUBE_H
#define YOUTUBE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

namespace QYouTube {
    class ResourcesRequest;
    class StreamsRequest;
}

class YouTube : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl.YouTube")
#endif

public:
    explicit YouTube(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new YouTube; }
    inline QString iconName() const { return QString("youtube.jpg"); }
    inline QString serviceName() const { return QString("YouTube"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private slots:
    void onResourcesRequestFinished();
    void onStreamsRequestFinished();

signals:
    void currentOperationCancelled();
    
private:
    QYouTube::ResourcesRequest *m_resourcesRequest;
    QYouTube::StreamsRequest *m_streamsRequest;
};

#endif // YOUTUBE_H
