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

#ifndef VIDEOBAM_H
#define VIDEOBAM_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class VideoBam : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl.VideoBam")
#endif

public:
    explicit VideoBam(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new VideoBam; }
    inline QString iconName() const { return QString("videobam.jpg"); }
    inline QString serviceName() const { return QString("VideoBam"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    bool cancelCurrentOperation();

private:
    void getDownloadLink(const QUrl &url);
    void startWait(int msecs);

private slots:
    void checkUrlIsValid();
    void onWebPageLoaded();
    void checkDownloadLink();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QTimer *m_waitTimer;
    int m_waitTime;
};

#endif // VIDEOBAM_H
