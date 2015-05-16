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

#ifndef RECORDEDBABES_H
#define RECORDEDBABES_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class RecordedBabes : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl.RecordedBabes")
#endif

public:
    explicit RecordedBabes(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new RecordedBabes; }
    inline QString iconName() const { return QString("recordedbabes.jpg"); }
    inline QString serviceName() const { return QString("RecordedBabes"); }
    virtual QRegExp urlPattern() const;
    virtual bool urlSupported(const QUrl &url) const;
    virtual void checkUrl(const QUrl &url);
    virtual void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private slots:
    void checkUrlIsValid();
    void parseVideoPage();

signals:
    void currentOperationCancelled();
};

#endif // RECORDEDBABES_H
