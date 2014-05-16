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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QQueue>
#include <QStringList>

class ServicePlugin;
class DecaptchaPlugin;
class RecaptchaPlugin;
class QUrl;

class PluginManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int progress
               READ progress
               NOTIFY progressChanged)

public:
    inline int progress() const { return m_progress; }

    bool servicePluginExists(const QString &serviceName) const;
    bool servicePluginExists(const QUrl &url) const;
    ServicePlugin* getServicePlugin(const QString &serviceName) const;
    ServicePlugin* getServicePlugin(const QUrl &url) const;
    ServicePlugin* createServicePlugin(const QString &serviceName);
    ServicePlugin* createServicePlugin(const QUrl &url);
    inline QList<ServicePlugin*> servicePlugins() const { return m_services; }
    inline QStringList servicePluginNames() const { return m_serviceNames; }

    DecaptchaPlugin* getDecaptchaPlugin(const QString &serviceName) const;
    DecaptchaPlugin* createDecaptchaPlugin(const QString &serviceName);
    inline QList<DecaptchaPlugin*> decaptchaPlugins() const { return m_decaptchaServices; }
    inline QStringList decaptchaPluginNames() const { return m_decaptchaNames; }

    RecaptchaPlugin* getRecaptchaPlugin(const QString &serviceName) const;
    RecaptchaPlugin* createRecaptchaPlugin(const QString &serviceName);
    inline QList<RecaptchaPlugin*> recaptchaPlugins() const { return m_recaptchaServices; }
    inline QStringList recaptchaPluginNames() const { return m_recaptchaNames; }

    static PluginManager* instance();

public slots:
    void loadPlugins();
    
private:
    PluginManager();
    ~PluginManager();

    void loadServicePlugins();
    void loadDecaptchaPlugins();
    void loadRecaptchaPlugins();
    void loginToAccounts();

private slots:
    void onAccountLogin(bool ok);

signals:
    void busy(const QString &message, int numberOfOperations);
    void progressChanged(int progress);
    void pluginsReady();
    
private:
    static PluginManager *self;

    QList<ServicePlugin*> m_services;
    QStringList m_serviceNames;

    QList<DecaptchaPlugin*> m_decaptchaServices;
    QStringList m_decaptchaNames;

    QList<RecaptchaPlugin*> m_recaptchaServices;
    QStringList m_recaptchaNames;

    QQueue<ServicePlugin*> m_loginQueue;

    int m_progress;
};

#endif // PLUGINMANAGER_H
