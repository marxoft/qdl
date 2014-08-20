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

#ifndef TWOSHARED_H
#define TWOSHARED_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class TwoShared : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit TwoShared(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new TwoShared; }
    inline QString iconName() const { return QString("twoshared.jpg"); }
    inline QString serviceName() const { return QString("2Shared"); }
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

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void updateWaitTime();
    void onWaitFinished();
    void getDownloadLink();
    void checkDownloadLink();

signals:
    void currentOperationCancelled();

private:
    QUrl m_downloadUrl;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // TWOSHARED_H
