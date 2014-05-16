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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "enums.h"
#include <QSettings>

class QNetworkProxy;

class Settings : public QSettings
{
    Q_OBJECT

    Q_PROPERTY(bool startTransfersAutomatically
               READ startTransfersAutomatically
               WRITE setStartTransfersAutomatically
               NOTIFY startTransfersAutomaticallyChanged)
    Q_PROPERTY(QString downloadPath
               READ downloadPath
               WRITE setDownloadPath
               NOTIFY downloadPathChanged)
    Q_PROPERTY(QString language
               READ language
               WRITE setLanguage
               NOTIFY languageChanged)
    Q_PROPERTY(int maximumConcurrentTransfers
               READ maximumConcurrentTransfers
               WRITE setMaximumConcurrentTransfers
               NOTIFY maximumConcurrentTransfersChanged)
    Q_PROPERTY(int maximumConnectionsPerTransfer
               READ maximumConnectionsPerTransfer
               WRITE setMaximumConnectionsPerTransfer
               NOTIFY maximumConnectionsPerTransferChanged)
    Q_PROPERTY(int downloadRateLimit
               READ downloadRateLimit
               WRITE setDownloadRateLimit
               NOTIFY downloadRateLimitChanged)
    Q_PROPERTY(QString defaultCategory
               READ defaultCategory
               WRITE setDefaultCategory
               NOTIFY defaultCategoryChanged)
    Q_PROPERTY(bool monitorClipboard
               READ monitorClipboard
               WRITE setMonitorClipboard
               NOTIFY monitorClipboardChanged)
    Q_PROPERTY(QString decaptchaService
               READ decaptchaService
               WRITE setDecaptchaService
               NOTIFY decaptchaServiceChanged)
    Q_PROPERTY(ScreenOrientation::Orientation screenOrientation
               READ screenOrientation
               WRITE setScreenOrientation
               NOTIFY screenOrientationChanged)
    Q_PROPERTY(bool extractDownloadedArchives
               READ extractDownloadedArchives
               WRITE setExtractDownloadedArchives
               NOTIFY extractDownloadedArchivesChanged)
    Q_PROPERTY(bool createSubfolderForArchives
               READ createSubfolderForArchives
               WRITE setCreateSubfolderForArchives
               NOTIFY createSubfolderForArchivesChanged)
    Q_PROPERTY(bool deleteExtractedArchives
               READ deleteExtractedArchives
               WRITE setDeleteExtractedArchives
               NOTIFY deleteExtractedArchivesChanged)
    Q_PROPERTY(NetworkProxyType::ProxyType networkProxyType
               READ networkProxyType
               WRITE setNetworkProxyType
               NOTIFY networkProxyTypeChanged)
    Q_PROPERTY(QString networkProxyHostName
               READ networkProxyHostName
               WRITE setNetworkProxyHostName
               NOTIFY networkProxyHostNameChanged)
    Q_PROPERTY(quint16 networkProxyPort
               READ networkProxyPort
               WRITE setNetworkProxyPort
               NOTIFY networkProxyPortChanged)
    Q_PROPERTY(QString networkProxyUser
               READ networkProxyUser
               WRITE setNetworkProxyUser
               NOTIFY networkProxyUserChanged)
    Q_PROPERTY(QString networkProxyPassword
               READ networkProxyPassword
               WRITE setNetworkProxyPassword
               NOTIFY networkProxyPasswordChanged)
#ifdef WEB_INTERFACE
    Q_PROPERTY(bool enableWebInterface
               READ enableWebInterface
               WRITE setEnableWebInterface
               NOTIFY enableWebInterfaceChanged)
    Q_PROPERTY(quint16 webInterfacePort
               READ webInterfacePort
               WRITE setWebInterfacePort
               NOTIFY webInterfacePortChanged)
    Q_PROPERTY(QString webInterfaceTheme
               READ webInterfaceTheme
               WRITE setWebInterfaceTheme
               NOTIFY webInterfaceThemeChanged)
#endif

public:
    inline bool startTransfersAutomatically() const { return m_automatic; }
    inline QString downloadPath() const { return m_path; }
    inline QString language() const { return m_language; }
    inline int maximumConcurrentTransfers() const { return m_maxConcurrent; }
    inline int maximumConnectionsPerTransfer() const { return m_maxConnections; }
    inline int downloadRateLimit() const { return m_rateLimit; }
    inline QString defaultCategory() const { return m_category; }
    inline bool monitorClipboard() const { return m_monitorClipboard; }
    inline QString decaptchaService() const { return m_decaptchaService; }
    inline ScreenOrientation::Orientation screenOrientation() const { return m_orientation; }
    inline bool extractDownloadedArchives() const { return m_extractArchives; }
    inline bool createSubfolderForArchives() const { return m_archiveSubfolders; }
    inline bool deleteExtractedArchives() const { return m_deleteArchives; }
    inline NetworkProxyType::ProxyType networkProxyType() const { return m_proxyType; }
    inline QString networkProxyHostName() const { return m_proxyHost; }
    inline quint16 networkProxyPort() const { return m_proxyPort; }
    inline QString networkProxyUser() const { return m_proxyUser; }
    inline QString networkProxyPassword() const { return m_proxyPass; }
#ifdef WEB_INTERFACE
    inline bool enableWebInterface() const { return m_enableWebIf; }
    inline quint16 webInterfacePort() const { return m_webIfPort; }
    inline QString webInterfaceTheme() const { return m_webIfTheme; }
#endif
    Q_INVOKABLE QVariant setting(const QString &key) const;

    static Settings* instance();

public slots:
    void saveSettings();
    void restoreSettings();

    void setStartTransfersAutomatically(bool automatic);
    void setDownloadPath(const QString &path);
    void setLanguage(const QString &lang);
    void setMaximumConcurrentTransfers(int maximum);
    void setMaximumConnectionsPerTransfer(int maximum);
    void setDownloadRateLimit(int limit);
    void setDefaultCategory(const QString &category);
    void setMonitorClipboard(bool monitor);
    void setDecaptchaService(const QString &service);
    void setScreenOrientation(ScreenOrientation::Orientation orientation);
    void setExtractDownloadedArchives(bool extract);
    void setCreateSubfolderForArchives(bool createSubfolder);
    void setDeleteExtractedArchives(bool deleteExtracted);
    void setNetworkProxyType(NetworkProxyType::ProxyType proxyType);
    void setNetworkProxyHostName(const QString &hostName);
    void setNetworkProxyPort(quint16 port);
    void setNetworkProxyUser(const QString &user);
    void setNetworkProxyPassword(const QString &password);
    void setNetworkProxy();
#ifdef WEB_INTERFACE
    void setEnableWebInterface(bool enable);
    void setWebInterfacePort(quint16 port);
    void setWebInterfaceTheme(const QString &theme);
#endif
    void setSetting(const QString &key, const QVariant &value);

private:
    Settings();
    ~Settings();

private slots:
    void onNetworkProxyAuthenticationRequested(const QNetworkProxy &proxy, QAuthenticator *authenticator);

signals:
    void startTransfersAutomaticallyChanged(bool automatic);
    void downloadPathChanged(const QString &path);
    void languageChanged(const QString &language);
    void maximumConcurrentTransfersChanged(int oldMaximum, int newMaximum);
    void maximumConnectionsPerTransferChanged(int oldMaximum, int newMaximum);
    void downloadRateLimitChanged(int limit);
    void defaultCategoryChanged(const QString &category);
    void monitorClipboardChanged(bool monitor);
    void decaptchaServiceChanged(const QString &service);
    void screenOrientationChanged(ScreenOrientation::Orientation orientation);
    void extractDownloadedArchivesChanged(bool extract);
    void createSubfolderForArchivesChanged(bool createSubfolder);
    void deleteExtractedArchivesChanged(bool deleteExtracted);
    void networkProxyTypeChanged(NetworkProxyType::ProxyType proxyType);
    void networkProxyHostNameChanged(const QString &hostName);
    void networkProxyPortChanged(quint16 port);
    void networkProxyUserChanged(const QString &user);
    void networkProxyPasswordChanged(const QString &password);
#ifdef WEB_INTERFACE
    void enableWebInterfaceChanged(bool enable);
    void webInterfacePortChanged(quint16 port);
    void webInterfaceThemeChanged(const QString &theme);
#endif

private:
    static Settings *self;

    bool m_automatic;
    QString m_path;
    int m_maxConcurrent;
    int m_maxConnections;
    int m_rateLimit;
    QString m_category;
    QString m_language;
    bool m_monitorClipboard;
    QString m_decaptchaService;
    ScreenOrientation::Orientation m_orientation;
    bool m_extractArchives;
    bool m_archiveSubfolders;
    bool m_deleteArchives;
    NetworkProxyType::ProxyType m_proxyType;
    QString m_proxyTypeString;
    QString m_proxyHost;
    quint16 m_proxyPort;
    QString m_proxyUser;
    QString m_proxyPass;
#ifdef WEB_INTERFACE
    bool m_enableWebIf;
    quint16 m_webIfPort;
    QString m_webIfTheme;
#endif
};

#endif // SETTINGS_H
