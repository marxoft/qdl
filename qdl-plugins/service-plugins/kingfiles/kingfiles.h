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

#ifndef KINGFILES_H
#define KINGFILES_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Kingfiles : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl.Kingfiles")
#endif

public:
    explicit Kingfiles(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Kingfiles; }
    inline QString iconName() const { return QString("kingfiles.jpg"); }
    inline QString serviceName() const { return QString("Kingfiles"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline QString recaptchaServiceName() const { return QString("Kingfiles"); }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void getWaitTime();
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkWaitTime();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void onWaitFinished();
    void downloadCaptcha();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_fileId;
    QString m_fileName;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // KINGFILES_H
