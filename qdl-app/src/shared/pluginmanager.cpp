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

#include "pluginmanager.h"
#include "../interfaces/serviceplugin.h"
#include "../interfaces/decaptchaplugin.h"
#include "../interfaces/recaptchaplugin.h"
#include "database.h"
#include "networkaccessmanager.h"
#include <QPluginLoader>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

#if (defined Q_OS_SYMBIAN)
#define LIB_EXT "*.qtplugin"
#elif (defined Q_OS_UNIX)
#define LIB_EXT "*.so"
#else
#define LIB_EXT "*.dll"
#endif

PluginManager* PluginManager::self = 0;

PluginManager::PluginManager() :
    QObject(),
    m_progress(0)
{
    if (!self) {
        self = this;
    }
}

PluginManager::~PluginManager() {}

PluginManager* PluginManager::instance() {
    return !self ? new PluginManager : self;
}

void PluginManager::loadPlugins() {
    this->loadDecaptchaPlugins();
    this->loadRecaptchaPlugins();
    this->loadServicePlugins();
}

void PluginManager::loadServicePlugins() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("service_plugins");

    foreach(QString fileName, dir.entryList(QStringList() << LIB_EXT, QDir::Files)) {
        QPluginLoader pluginLoader(dir.absoluteFilePath(fileName));
        QObject *instance = pluginLoader.instance();

        if (ServiceInterface* serviceIf = qobject_cast<ServiceInterface*>(instance)) {
            if (ServicePlugin *plugin = serviceIf->getServicePlugin()) {
                plugin->setNetworkAccessManager(NetworkAccessManager::instance());
                m_services.append(plugin);
                m_serviceNames.append(plugin->serviceName());

                if (plugin->loginSupported()) {
                    m_loginQueue.enqueue(plugin);
                }
            }
        }
    }

    if (!m_loginQueue.isEmpty()) {
        emit busy(tr("Loading plugins"), m_loginQueue.size());
        this->loginToAccounts();
    }
    else {
        emit pluginsReady();
    }
}

void PluginManager::loadDecaptchaPlugins() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("decaptcha_plugins");

    foreach(QString fileName, dir.entryList(QStringList() << LIB_EXT, QDir::Files)) {
        QPluginLoader pluginLoader(dir.absoluteFilePath(fileName));
        QObject *instance = pluginLoader.instance();

        if (DecaptchaInterface* decaptchaIf = qobject_cast<DecaptchaInterface*>(instance)) {
            if (DecaptchaPlugin *plugin = decaptchaIf->getDecaptchaPlugin()) {
                plugin->setNetworkAccessManager(NetworkAccessManager::instance());
                QPair<QString, QString> account = Database::instance()->getAccount(plugin->serviceName());

                if ((!account.first.isEmpty()) && (!account.second.isEmpty())) {
                    plugin->login(account.first, account.second);
                }

                m_decaptchaServices.append(plugin);
                m_decaptchaNames.append(plugin->serviceName());
            }
        }
    }
}

void PluginManager::loadRecaptchaPlugins() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("recaptcha_plugins");

    foreach(QString fileName, dir.entryList(QStringList() << LIB_EXT, QDir::Files)) {
        QPluginLoader pluginLoader(dir.absoluteFilePath(fileName));
        QObject *instance = pluginLoader.instance();

        if (RecaptchaInterface* recaptchaIf = qobject_cast<RecaptchaInterface*>(instance)) {
            if (RecaptchaPlugin *plugin = recaptchaIf->getRecaptchaPlugin()) {
                plugin->setNetworkAccessManager(NetworkAccessManager::instance());
                m_recaptchaServices.append(plugin);
                m_recaptchaNames.append(plugin->serviceName());
            }
        }
    }
}

void PluginManager::loginToAccounts() {
    ServicePlugin *plugin = m_loginQueue.first();
    QPair<QString, QString> account = Database::instance()->getAccount(plugin->serviceName());

    if ((!account.first.isEmpty()) && (!account.second.isEmpty())) {
        plugin->login(account.first, account.second);
        this->connect(plugin, SIGNAL(loggedIn(bool)), this, SLOT(onAccountLogin(bool)));
    }
    else {
        emit progressChanged(m_progress++);
        m_loginQueue.dequeue();

        if (!m_loginQueue.isEmpty()) {
            this->loginToAccounts();
        }
        else {
            emit pluginsReady();
        }
    }
}

void PluginManager::onAccountLogin(bool ok) {
    emit progressChanged(m_progress++);

    if (ServicePlugin *plugin = m_loginQueue.dequeue()) {
        this->disconnect(plugin, SIGNAL(loggedIn(bool)), this, SLOT(onAccountLogin(bool)));

        if (ok) {
            qDebug() << "Login successful: " + plugin->serviceName();
        }
        else {
            qDebug() << "Login failed: " + plugin->serviceName();
        }
    }

    if (!m_loginQueue.isEmpty()) {
        this->loginToAccounts();
    }
    else {
        emit pluginsReady();
    }
}

bool PluginManager::servicePluginExists(const QString &serviceName) const {
    for (int i = 0; i < m_services.size(); i++) {
        if (m_services.at(i)->serviceName() == serviceName) {
            return true;
        }
    }

    return false;
}

bool PluginManager::servicePluginExists(const QUrl &url) const {
    for (int i = 0; i < m_services.size(); i++) {
        if (m_services.at(i)->urlSupported(url)) {
            return true;
        }
    }

    return false;
}

ServicePlugin* PluginManager::getServicePlugin(const QString &serviceName) const {
    for (int i = 0; i < m_services.size(); i++) {
        if (m_services.at(i)->serviceName() == serviceName) {
            return m_services.at(i);
        }
    }

    return 0;
}

ServicePlugin* PluginManager::getServicePlugin(const QUrl &url) const {
    for (int i = 0; i < m_services.size(); i++) {
        if (m_services.at(i)->urlSupported(url)) {
            return m_services.at(i);
        }
    }

    return 0;
}

ServicePlugin* PluginManager::createServicePlugin(const QString &serviceName) {
    for (int i = 0; i < m_services.size(); i++) {
        if (m_services.at(i)->serviceName() == serviceName) {
            if (ServicePlugin *plugin = m_services.at(i)->createServicePlugin()) {
                plugin->setNetworkAccessManager(NetworkAccessManager::instance());
                return plugin;
            }
            else {
                return 0;
            }
        }
    }

    return 0;
}

ServicePlugin* PluginManager::createServicePlugin(const QUrl &url) {
    for (int i = 0; i < m_services.size(); i++) {
        if (m_services.at(i)->urlSupported(url)) {
            if (ServicePlugin *plugin = m_services.at(i)->createServicePlugin()) {
                plugin->setNetworkAccessManager(NetworkAccessManager::instance());
                return plugin;
            }
            else {
                return 0;
            }
        }
    }

    return 0;
}

DecaptchaPlugin* PluginManager::getDecaptchaPlugin(const QString &serviceName) const {
    for (int i = 0; i < m_decaptchaServices.size(); i++) {
        if (m_decaptchaServices.at(i)->serviceName() == serviceName) {
            return m_decaptchaServices.at(i);
        }
    }

    return 0;
}

DecaptchaPlugin* PluginManager::createDecaptchaPlugin(const QString &serviceName) {
    for (int i = 0; i < m_decaptchaServices.size(); i++) {
        if (m_decaptchaServices.at(i)->serviceName() == serviceName) {
            if (DecaptchaPlugin *plugin = m_decaptchaServices.at(i)->createDecaptchaPlugin()) {
                plugin->setNetworkAccessManager(NetworkAccessManager::instance());
                plugin->login(m_decaptchaServices.at(i)->username(), m_decaptchaServices.at(i)->password());
                this->connect(m_decaptchaServices.at(i), SIGNAL(credentialsChanged(QString,QString)), plugin, SLOT(login(QString,QString)));
                return plugin;
            }
            else {
                return 0;
            }
        }
    }

    return 0;
}

RecaptchaPlugin* PluginManager::getRecaptchaPlugin(const QString &serviceName) const {
    for (int i = 0; i < m_recaptchaServices.size(); i++) {
        if (m_recaptchaServices.at(i)->serviceName() == serviceName) {
            return m_recaptchaServices.at(i);
        }
    }

    return 0;
}

RecaptchaPlugin* PluginManager::createRecaptchaPlugin(const QString &serviceName) {
    for (int i = 0; i < m_recaptchaServices.size(); i++) {
        if (m_recaptchaServices.at(i)->serviceName() == serviceName) {
            if (RecaptchaPlugin *plugin = m_recaptchaServices.at(i)->createRecaptchaPlugin()) {
                plugin->setNetworkAccessManager(NetworkAccessManager::instance());
                return plugin;
            }
            else {
                return 0;
            }
        }
    }

    return 0;
}
