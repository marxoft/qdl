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

#ifndef GIGAPETA_H
#define GIGAPETA_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class GigaPeta : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit GigaPeta(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new GigaPeta; }
    inline QString serviceName() const { return QString("GigaPeta"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // GIGAPETA_H
