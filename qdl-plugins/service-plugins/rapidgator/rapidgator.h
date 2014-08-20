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

#ifndef RAPIDGATOR_H
#define RAPIDGATOR_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class RapidGator : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit RapidGator(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new RapidGator; }
    inline QString iconName() const { return QString("rapidgator.jpg"); }
    inline QString serviceName() const { return QString("RapidGator"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaServiceName() const { return QString("SolveMedia"); }
    inline QString recaptchaKey() const { return QString("oy3wKTaFP368dkJiGUqOVjBR2rOOR7GR"); }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void startWait(int msecs);
    void getSessionId();

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void checkSessionId();
    void checkDownloadLink();
    void onWaitFinished();
    void getDownloadLink();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_fileId;
    int m_secs;
    QString m_sessionId;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // RAPIDGATOR_H
