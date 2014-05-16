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

#include "settings.h"
#include "definitions.h"
#include <qplatformdefs.h>
#include <QDir>
#include <QNetworkProxy>
#include <QAuthenticator>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

Settings* Settings::self = 0;

Settings::Settings() :
    QSettings(QString("QDL"), QString("QDL")),
    m_automatic(true),
    m_maxConcurrent(1),
    m_maxConnections(1),
    m_rateLimit(0),
    m_category(tr("Default")),
    m_monitorClipboard(false),
    m_orientation(ScreenOrientation::Automatic),
    m_extractArchives(true),
    m_archiveSubfolders(true),
    m_deleteArchives(true),
    m_proxyType(NetworkProxyType::HttpProxy),
    m_proxyPort(80)
  #ifdef WEB_INTERFACE
    ,m_enableWebIf(false),
    m_webIfPort(8080),
    m_webIfTheme("Default")
  #endif
{
    if (!self) {
        self = this;
    }

    this->restoreSettings();
}

Settings::~Settings() {
    this->saveSettings();
}

Settings* Settings::instance() {
    return !self ? new Settings : self;
}

void Settings::restoreSettings() {
    this->beginGroup("Transfers");
    this->setStartTransfersAutomatically(this->value("startTransfersAutomatically", true).toBool());

#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN)
    this->setDownloadPath(this->value("downloadPath", QString("/home/user/MyDocs/QDL/")).toString());
#elif (defined Q_OS_SYMBIAN)
    this->setDownloadPath(this->value("downloadPath", QString("E:/QDL/")).toString());
#elif QT_VERSION >= 0x050000
    this->setDownloadPath(this->value("downloadPath", QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/QDL/")).toString());
#else
    this->setDownloadPath(this->value("downloadPath", QDesktopServices::storageLocation(QDesktopServices::HomeLocation).append("/QDL/")).toString());
#endif

    this->setMaximumConcurrentTransfers(this->value("maximumConcurrentTransfers", 1).toInt());
    this->setMaximumConnectionsPerTransfer(this->value("maximumConnectionsPerTransfer", 1).toInt());
    this->setDownloadRateLimit(this->value("downloadRateLimit", 0).toInt());
    this->setDefaultCategory(this->value("defaultCategory", tr("Default")).toString());
    this->setExtractDownloadedArchives(this->value("extractDownloadedArchives", true).toBool());
    this->setCreateSubfolderForArchives(this->value("createSubfolderForArchives", true).toBool());
    this->setDeleteExtractedArchives(this->value("deleteExtractedArchives", true).toBool());
    this->endGroup();

    this->beginGroup("Decaptcha");
    this->setDecaptchaService(this->value("decaptchaService", QString()).toString());
    this->endGroup();

    this->beginGroup("Network");
    this->setNetworkProxyType(static_cast<NetworkProxyType::ProxyType>(this->value("networkProxyType", NetworkProxyType::ProxyType(NetworkProxyType::HttpProxy)).toInt()));
    this->setNetworkProxyHostName(this->value("networkProxyHostName", QString()).toString());
    this->setNetworkProxyPort(this->value("networkProxyPort", 80).toUInt());
    this->setNetworkProxyUser(this->value("networkProxyUser", QString()).toString());
    this->setNetworkProxyPassword(QString(QByteArray::fromBase64(this->value("networkProxyPassword", QByteArray()).toByteArray())));
    this->setNetworkProxy();
    this->endGroup();

#ifdef WEB_INTERFACE
    this->beginGroup("Interfaces");
    this->setEnableWebInterface(this->value("enableWebInterface", false).toBool());
    this->setWebInterfacePort(this->value("webInterfacePort", 8080).toLongLong());
    this->setWebInterfaceTheme(this->value("webInterfaceTheme", QString("Default")).toString());
    this->endGroup();
#endif

    this->beginGroup("Other");
    this->setLanguage(this->value("language", QString("en")).toString());
    this->setMonitorClipboard(this->value("monitorClipboard", false).toBool());
    this->setScreenOrientation(static_cast<ScreenOrientation::Orientation>(this->value("screenOrientation", ScreenOrientation::Orientation(ScreenOrientation::Automatic)).toInt()));
    this->endGroup();
}

void Settings::saveSettings() {
    this->beginGroup("Transfers");
    this->setValue("startTransfersAutomatically", this->startTransfersAutomatically());
    this->setValue("downloadPath", this->downloadPath());
    this->setValue("maximumConcurrentTransfers", this->maximumConcurrentTransfers());
    this->setValue("maximumConnectionsPerTransfer", this->maximumConnectionsPerTransfer());
    this->setValue("downloadRateLimit", this->downloadRateLimit());
    this->setValue("defaultCategory", this->defaultCategory());
    this->setValue("extractDownloadedArchives", this->extractDownloadedArchives());
    this->setValue("createSubfolderForArchives", this->createSubfolderForArchives());
    this->setValue("deleteExtractedArchives", this->extractDownloadedArchives());
    this->endGroup();

    this->beginGroup("Decaptcha");
    this->setValue("decaptchaService", this->decaptchaService());
    this->endGroup();

    this->beginGroup("Network");
    this->setValue("networkProxyType", NetworkProxyType::ProxyType(this->networkProxyType()));
    this->setValue("networkProxyHostName", this->networkProxyHostName());
    this->setValue("networkProxyPort", this->networkProxyPort());
    this->setValue("networkProxyUser", this->networkProxyUser());
    this->setValue("networkProxyPassword", this->networkProxyPassword().toUtf8().toBase64());
    this->endGroup();

#ifdef WEB_INTERFACE
    this->beginGroup("Interfaces");
    this->setValue("enableWebInterface", this->enableWebInterface());
    this->setValue("webInterfacePort", this->webInterfacePort());
    this->setValue("webInterfaceTheme", this->webInterfaceTheme());
    this->endGroup();
#endif

    this->beginGroup("Other");
    this->setValue("language", this->language());
    this->setValue("monitorClipboard", this->monitorClipboard());
    this->setValue("screenOrientation", ScreenOrientation::Orientation(this->screenOrientation()));
    this->endGroup();
}

QVariant Settings::setting(const QString &key) const {
    return this->value(key);
}

void Settings::setSetting(const QString &key, const QVariant &value) {
    this->setValue(key, value);
}

void Settings::setStartTransfersAutomatically(bool automatic) {
    if (automatic != this->startTransfersAutomatically()) {
        m_automatic = automatic;
        emit startTransfersAutomaticallyChanged(automatic);
    }
}

void Settings::setDownloadPath(const QString &path) {
    if (path != this->downloadPath()) {
        m_path = path.endsWith('/') ? path : path + '/';
        emit downloadPathChanged(this->downloadPath());
    }
}

void Settings::setLanguage(const QString &lang) {
    if (lang != this->language()) {
        m_language = lang;
        emit languageChanged(lang);
    }
}

void Settings::setMaximumConcurrentTransfers(int maximum) {
    int oldMaximum = this->maximumConcurrentTransfers();

    if ((maximum != oldMaximum) && (maximum > 0) && (maximum <= MAX_CONCURRENT_TRANSFERS)) {
        m_maxConcurrent = maximum;
        emit maximumConcurrentTransfersChanged(oldMaximum, maximum);
    }
}

void Settings::setMaximumConnectionsPerTransfer(int maximum) {
    int oldMaximum = this->maximumConnectionsPerTransfer();

    if ((maximum != oldMaximum) && (maximum > 0) && (maximum <= MAX_CONNECTIONS)) {
        m_maxConnections = maximum;
        emit maximumConcurrentTransfersChanged(oldMaximum, maximum);
    }
}

void Settings::setDownloadRateLimit(int limit) {
    if (limit != this->downloadRateLimit()) {
        m_rateLimit = limit;
        emit downloadRateLimitChanged(limit);
    }
}

void Settings::setDefaultCategory(const QString &category) {
    if (category != this->defaultCategory()) {
        m_category = category;
        emit defaultCategoryChanged(category);
    }
}

void Settings::setMonitorClipboard(bool monitor) {
    if (monitor != this->monitorClipboard()) {
        m_monitorClipboard = monitor;
        emit monitorClipboardChanged(monitor);
    }
}

void Settings::setDecaptchaService(const QString &service) {
    if (service != this->decaptchaService()) {
        m_decaptchaService = service;
        emit decaptchaServiceChanged(service);
    }
}

void Settings::setScreenOrientation(ScreenOrientation::Orientation orientation) {
    if (orientation != this->screenOrientation()) {
        m_orientation = orientation;
        emit screenOrientationChanged(orientation);
    }
}

void Settings::setExtractDownloadedArchives(bool extract) {
    if (extract != this->extractDownloadedArchives()) {
        m_extractArchives = extract;
        emit extractDownloadedArchivesChanged(extract);
    }
}

void Settings::setCreateSubfolderForArchives(bool createSubfolder) {
    if (createSubfolder != this->createSubfolderForArchives()) {
        m_archiveSubfolders = createSubfolder;
        emit createSubfolderForArchivesChanged(createSubfolder);
    }
}

void Settings::setDeleteExtractedArchives(bool deleteExtracted) {
    if (deleteExtracted != this->deleteExtractedArchives()) {
        m_deleteArchives = deleteExtracted;
        emit deleteExtractedArchivesChanged(deleteExtracted);
    }
}

void Settings::setNetworkProxyType(NetworkProxyType::ProxyType proxyType) {
    if (proxyType != this->networkProxyType()) {
        m_proxyType = proxyType;
        emit networkProxyTypeChanged(proxyType);
    }
}

void Settings::setNetworkProxyHostName(const QString &hostName) {
    if (hostName != this->networkProxyHostName()) {
        m_proxyHost = hostName;
        emit networkProxyHostNameChanged(hostName);
    }
}

void Settings::setNetworkProxyPort(quint16 port) {
    if (port != this->networkProxyPort()) {
        m_proxyPort = port;
        emit networkProxyPortChanged(port);
    }
}

void Settings::setNetworkProxyUser(const QString &user) {
    if (user != this->networkProxyUser()) {
        m_proxyUser = user;
        emit networkProxyUserChanged(user);
    }
}

void Settings::setNetworkProxyPassword(const QString &password) {
    if (password != this->networkProxyPassword()) {
        m_proxyPass = password;
        emit networkProxyPasswordChanged(password);
    }
}

void Settings::setNetworkProxy() {
    QNetworkProxy proxy;

    if (!this->networkProxyHostName().isEmpty()) {
        proxy.setHostName(this->networkProxyHostName());
        proxy.setType(QNetworkProxy::ProxyType(this->networkProxyType()));

        if (this->networkProxyPort() > 0) {
            proxy.setPort(this->networkProxyPort());
        }
        else {
            proxy.setPort(80);
        }

        if ((!this->networkProxyUser().isEmpty()) && (!this->networkProxyPassword().isEmpty())) {
            proxy.setUser(this->networkProxyUser());
            proxy.setPassword(this->networkProxyPassword());
        }
    }

    QNetworkProxy::setApplicationProxy(proxy);
}

void Settings::onNetworkProxyAuthenticationRequested(const QNetworkProxy &proxy, QAuthenticator *authenticator) {
    Q_UNUSED(proxy)

    authenticator->setUser(this->networkProxyUser());
    authenticator->setPassword(this->networkProxyPassword());
}

#ifdef WEB_INTERFACE
void Settings::setEnableWebInterface(bool enable) {
    if (enable != this->enableWebInterface()) {
        m_enableWebIf = enable;
        emit enableWebInterfaceChanged(enable);
    }
}

void Settings::setWebInterfacePort(quint16 port) {
    if (port != this->webInterfacePort()) {
        m_webIfPort = port;
        emit webInterfacePortChanged(port);
    }
}

void Settings::setWebInterfaceTheme(const QString &theme) {
    if (theme != this->webInterfaceTheme()) {
        m_webIfTheme = theme;
        emit webInterfaceThemeChanged(theme);
    }
}
#endif
