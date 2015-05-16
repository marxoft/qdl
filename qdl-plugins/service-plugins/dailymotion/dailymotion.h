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

#ifndef DAILYMOTION_H
#define DAILYMOTION_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include "serviceplugin.h"

class Dailymotion : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl.Dailymotion")
#endif

public:
    explicit Dailymotion(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Dailymotion; }
    inline QString iconName() const { return QString("dailymotion.jpg"); }
    inline QString serviceName() const { return QString("Dailymotion"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &webUrl);
    void getDownloadRequest(const QUrl &webUrl);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    inline QString errorString() const { return m_errorString; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private:
    void checkPlaylistVideoUrls(const QUrl &url);
    void getVideoUrl(const QUrl &url);
    inline void setErrorString(const QString &errorString) { m_errorString = errorString; }

private slots:
    void checkUrlIsValid();
    void parseVideoPage();
    void checkVideoUrl();

signals:
    void currentOperationCancelled();

private:
    QStringList m_formatList;
    QString m_errorString;
};

#endif // DAILYMOTION_H
