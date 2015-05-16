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

#ifndef DATAFILE_H
#define DATAFILE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Datafile : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl.Datafile")
#endif

public:
    explicit Datafile(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Datafile; }
    inline QString iconName() const { return QString("datafile.jpg"); }
    inline QString serviceName() const { return QString("Datafile"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &webUrl);
    void getDownloadRequest(const QUrl &webUrl);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline QString errorString() const { return m_errorString; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void startWait(int msecs);
    inline void setErrorString(const QString &errorString) { m_errorString = errorString; }

private slots:
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void downloadCaptcha();
    void onWaitFinished();
    void checkLogin();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_fileId;
    QString m_captchaKey;
    QString m_errorString;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // DATAFILE_H
