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

#ifndef RECAPTCHAPLUGIN_H
#define RECAPTCHAPLUGIN_H

#include <QObject>
#include "recaptchainterface.h"

class QNetworkAccessManager;
class QUrl;
class QString;

class RecaptchaPlugin : public QObject, public RecaptchaInterface
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    enum ErrorType {
        CaptchaNotFound,
        ServiceUnavailable,
        Unauthorised,
        BadRequest,
        InternalError,
        NetworkError,
        UnknownError
    };

public:
    explicit RecaptchaPlugin(QObject *parent = 0) : QObject(parent), m_nam(0) {}
    virtual ~RecaptchaPlugin() {}
    inline RecaptchaPlugin* getRecaptchaPlugin() { return this; }
    virtual RecaptchaPlugin* createRecaptchaPlugin() = 0;
    inline QNetworkAccessManager *networkAccessManager() const { return m_nam; }
    inline void setNetworkAccessManager(QNetworkAccessManager *manager) { m_nam = manager; }
    virtual QString serviceName() const = 0;
    inline QString key() const { return m_key; }
    inline QString challenge() const { return m_challenge; }
    virtual void getCaptcha(const QString &key) = 0;
    virtual bool cancelCurrentOperation() = 0;

protected:
    void setKey(const QString &key) { m_key = key; }
    void setChallenge(const QString &challenge) { m_challenge = challenge; }

signals:
    void captchaReady(const QByteArray &imageData);
    void error(RecaptchaPlugin::ErrorType errorType);

protected:
    QNetworkAccessManager *m_nam;
    QString m_key;
    QString m_challenge;
};

#endif // RECAPTCHAPLUGIN_H
