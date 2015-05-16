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

#ifndef SHAREVNN_H
#define SHAREVNN_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class FShare : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl.FShare")
#endif

public:
    explicit FShare(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new FShare; }
    inline QString iconName() const { return QString("fshare.jpg"); }
    inline QString serviceName() const { return QString("FShare"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

private:
    void startWait(int msecs);
    void getDownloadLink();

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkDownloadLink();
    void updateWaitTime();
    void onWaitFinished();
    void startDownload();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QUrl m_downloadUrl;
    QString m_fileId;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // SHAREVNN_H
