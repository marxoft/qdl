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

#ifndef XERVER_H
#define XERVER_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Xerver : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Xerver(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Xerver; }
    inline QString iconName() const { return QString("xerver.jpg"); }
    inline QString serviceName() const { return QString("Xerver"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return false; }
    inline QString errorString() const { return m_errorString; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

private:
    void getWaitTime();
    void startWait(int msecs);
    inline void setErrorString(const QString &errorString) { m_errorString = errorString; }

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkWaitTime();
    void getDownloadLink();
    void checkDownloadLink();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_fileId;
    QString m_fileName;
    QString m_rand;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
    QString m_errorString;
};

#endif // XERVER_H
