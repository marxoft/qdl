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

#ifndef DEATHBYCAPTCHA_H
#define DEATHBYCAPTCHA_H

#include "decaptchaplugin.h"
#include "formpost.h"
#include <QObject>
#include <QUrl>

class DeathByCaptcha : public DecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(DecaptchaInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qdl.DeatchByCaptcha")
#endif

public:
    explicit DeathByCaptcha(QObject *parent = 0);
    DecaptchaPlugin* createDecaptchaPlugin() { return new DeathByCaptcha; }
    inline QString iconName() const { return QString("deathbycaptcha.jpg"); }
    inline QString serviceName() const { return QString("DeathByCaptcha"); }
    inline QString captchaId() const { return m_captchaId; }
    void getCaptchaResponse(const QByteArray &data);
    void reportIncorrectCaptchaResponse(const QString &id);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaSubmitted();
    void checkCaptchaStatus();
    void checkCaptchaStatusResponse();
    void onCaptchaReported();

signals:
    void currentOperationCancelled();

private:
    QUrl m_statusUrl;
    QString m_captchaId;
    FormPostPlugin *m_formPost;
};

#endif // DEATHBYCAPTCHA_H
