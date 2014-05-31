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

#ifndef TRANSFER_H
#define TRANSFER_H

#include "enums.h"
#include "file.h"
#include "../interfaces/serviceplugin.h"
#include "../interfaces/recaptchaplugin.h"
#include "../interfaces/decaptchaplugin.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QTime>
#include <QStringList>

class NetworkAccessManager;
class Connection;
class AudioConverter;
class ArchiveExtractor;

class Transfer : public QObject
{
    Q_OBJECT

    Q_ENUMS(Roles)

#ifdef QML_USER_INTERFACE
    Q_PROPERTY(QString id
               READ id
               CONSTANT)
    Q_PROPERTY(QUrl url
               READ url
               CONSTANT)
    Q_PROPERTY(QString fileName
               READ fileName
               CONSTANT)
    Q_PROPERTY(QString fileSuffix
               READ fileSuffix
               CONSTANT)
    Q_PROPERTY(QString iconFileName
               READ iconFileName
               CONSTANT)
    Q_PROPERTY(QString downloadPath
               READ downloadPath
               CONSTANT)
    Q_PROPERTY(QString serviceName
               READ serviceName
               CONSTANT)
    Q_PROPERTY(QString category
               READ category
               WRITE setCategory
               NOTIFY categoryChanged)
    Q_PROPERTY(Transfers::Priority priority
               READ priority
               WRITE setPriority
               NOTIFY priorityChanged)
    Q_PROPERTY(QString priorityString
               READ priorityString
               NOTIFY priorityChanged)
    Q_PROPERTY(qint64 size
               READ size
               NOTIFY sizeChanged)
    Q_PROPERTY(qint64 position
               READ position
               NOTIFY positionChanged)
    Q_PROPERTY(int progress
               READ progress
               NOTIFY progressChanged)
    Q_PROPERTY(int speed
               READ speed
               NOTIFY speedChanged)
    Q_PROPERTY(Transfers::Status status
               READ status
               NOTIFY statusChanged)
    Q_PROPERTY(QString statusString
               READ statusString
               NOTIFY statusInfoChanged)
    Q_PROPERTY(QString statusInfo
               READ statusInfo
               NOTIFY statusInfoChanged)
    Q_PROPERTY(int preferredConnections
               READ preferredConnections
               WRITE setPreferredConnections
               NOTIFY preferredConnectionsChanged)
    Q_PROPERTY(int maximumConnections
               READ maximumConnections
               CONSTANT)
    Q_PROPERTY(QString captchaFileName
               READ captchaFileName
               CONSTANT)
    Q_PROPERTY(int captchaTimeOut
               READ captchaTimeOut)
    Q_PROPERTY(QString captchaResponse
               READ captchaResponse
               WRITE submitCaptchaResponse)
    Q_PROPERTY(bool convertibleToAudio
               READ convertibleToAudio
               CONSTANT)
    Q_PROPERTY(bool convertToAudio
               READ convertToAudio
               WRITE setConvertToAudio
               NOTIFY convertToAudioChanged)
    Q_PROPERTY(bool downloadIsResumable
               READ downloadIsResumable
               CONSTANT)
    Q_PROPERTY(QString packageId
               READ packageId
               CONSTANT)
    Q_PROPERTY(QString packageName
               READ packageName
               CONSTANT)
    Q_PROPERTY(QString packageSuffix
               READ packageSuffix
               CONSTANT)
    Q_PROPERTY(int count
               READ count
               NOTIFY countChanged)
    Q_PROPERTY(bool expanded
               READ expanded
               WRITE setExpanded
               NOTIFY expandedChanged)
#endif

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ServiceNameRole,
        IconRole,
        CategoryRole,
        PriorityRole,
        PriorityStringRole,
        SizeRole,
        PositionRole,
        ProgressRole,
        StatusRole,
        StatusStringRole,
        ConvertibleToAudioRole,
        ConvertToAudioRole,
        PreferredConnectionsRole,
        MaximumConnectionsRole,
        DownloadResumableRole,
        CaptchaFileNameRole,
        CaptchaTimeOutRole,
        CaptchaResponseRole,
        TransferCountRole,
        IdRole,
        PackageIdRole,
        PackageNameRole,
        PackageStatusRole,
        RowNumberRole
#ifdef QML_USER_INTERFACE
        ,ExpandedRole
#endif
    };

    explicit Transfer(QObject *parent = 0);
    ~Transfer();

    Q_INVOKABLE QVariant data(int role) const;
    QMap<int, QVariant> itemData() const;
    Q_INVOKABLE QVariantMap itemDataWithRoleNames() const;
    Q_INVOKABLE bool setData(int role, const QVariant &value);

    Q_INVOKABLE bool match(int role, const QVariant &value);

    bool isValid() const;
    bool isPackage() const;

    int rowNumber() const;
    void setRowNumber(int row);

#ifdef QML_USER_INTERFACE
    bool expanded() const;
    void setExpanded(bool expanded);
#endif

    Transfer* parentTransfer() const;
    void setParentTransfer(Transfer *transfer);

    QString packageId() const;
    void setPackageId(const QString &id);

    QString packageName() const;
    void setPackageName(const QString &name);

    QString packageSuffix() const;
    void setPackageSuffix(const QString &suffix);

    Transfers::Status packageStatus() const;

    QString id() const;
    void setId(const QString &id);

    QUrl url() const;
    void setUrl(const QUrl &url);

    QString fileName() const;
    void setFileName(const QString &fileName);

    QString fileSuffix() const;

    QString downloadPath() const;
    void setDownloadPath(const QString &path);

    QString serviceName() const;
    void setServiceName(const QString &name);

    QString iconFileName() const;

    QPixmap icon() const;

    QString category() const;
    void setCategory(const QString &category);

    Transfers::Priority priority() const;
    void setPriority(Transfers::Priority priority);

    QString priorityString() const;

    qint64 size() const;
    void setSize(qint64 size);

    qint64 resumePosition() const;
    void setResumePosition(qint64 position);

    qint64 position() const;

    int progress() const;

    int speed() const;

    Transfers::Status status() const;

    QString statusString() const;

    QString statusInfo() const;

    bool convertibleToAudio() const;

    bool convertToAudio() const;
    void setConvertToAudio(bool convert);

    int preferredConnections() const;
    void setPreferredConnections(int pref, bool overrideGlobalSetting = true);

    int maximumConnections() const;

    int activeConnections() const;

    QString captchaFileName() const;

    int captchaTimeOut() const;

    QString captchaResponse() const;

    bool downloadIsResumable() const;

    int count() const;

    void createPackage();

    bool transferBelongsToPackage(Transfer *transfer) const;

    Transfer* childTransfer(int row) const;
    QList<Transfer*> childTransfers() const;
    void addChildTransfer(Transfer *transfer);
    void insertChildTransfer(int row, Transfer *transfer);
    Transfer* removeChildTransfer(int row);
    Transfer* removeChildTransfer(Transfer* transfer);

    QList<Connection*> connections() const;
    void restoreConnection(qint64 start, qint64 end);

public slots:
    void queue();
    void start();
    void pause();
    void cancel();

    void queuePackage();
    void pausePackage();
    void cancelPackage();
    
    bool submitCaptchaResponse(const QString &response);

private:
    void setIconFileName(const QString &fileName);
    void setProgress(int progress);
    void setStatus(Transfers::Status status);
    void setPackageStatus(Transfers::Status status);
    void setStatusInfo(const QString &info);
    void setMaximumConnections(int maximum);
    void loadConnections();
    void addConnection(qint64 start, qint64 end, bool startWhenAdded = true);
    void addConnections(int count = 1);
    void removeConnections(int count = 1);

private slots:
    void onRecaptchaPluginError(RecaptchaPlugin::ErrorType errorType);
    void onCaptchaReady(const QByteArray &imageData);
    void onDecaptchaPluginError(DecaptchaPlugin::ErrorType errorType);
    void onCaptchaResponseReady(const QString &response);
    void onServicePluginWaiting(int msecs);
    void onServicePluginStatusChanged(ServicePlugin::Status status);
    void onServicePluginError(ServicePlugin::ErrorType errorType);
    void onCaptchaRequired();
    void onCaptchaIncorrect();
    void onCaptchaRejectedByUser();
    void onDownloadRequestReady(const QNetworkRequest &request, const QByteArray &data = QByteArray());
    void onMetaInfoReady(MetaInfo info);
    void onDataAvailable(qint64 offset, const QByteArray &data);
    void onBytesDownloaded(qint64 bytes);
    void onFileError();
    void onConnectionStatusChanged(Transfers::Status status);
    void onConnectionCompleted(Connection *connection);
    void onFileWriteCompleted();
    void onMaximumConnectionsChanged(int oldMaximum, int newMaximum);
    void startAudioConversion();
    void onAudioConversionFinished();
    void onAudioConversionError();
    void extractArchive(const QString &password = QString());
    void onArchiveExtractionFinished();
    void onArchiveExtractionError();
    void moveFiles();
    void removeCaptchaFile();
    void removeFiles();

signals:
    void statusChanged(Transfers::Status status);
#ifdef QML_USER_INTERFACE
    void preferredConnectionsChanged();
    void categoryChanged();
    void priorityChanged();
    void sizeChanged();
    void progressChanged();
    void positionChanged();
    void speedChanged();
    void convertToAudioChanged();
    void statusInfoChanged();
    void countChanged();
    void expandedChanged();
#else
    void dataChanged(int role);
#endif

private:
    Transfer *m_parent;
    ServicePlugin *m_servicePlugin;
    RecaptchaPlugin *m_recaptchaPlugin;
    DecaptchaPlugin *m_decaptchaPlugin;
    NetworkAccessManager *m_nam;
    AudioConverter *m_converter;
    ArchiveExtractor *m_extractor;
    File m_file;
    QNetworkRequest m_request;
    QByteArray m_data;
    QString m_packageId;
    QString m_packageName;
    QString m_packageSuffix;
    Transfers::Status m_packageStatus;
    QString m_id;
    QUrl m_url;
    QString m_fileName;
    QString m_downloadPath;
    QString m_serviceName;
    QString m_iconFileName;
    QString m_category;
    Transfers::Priority m_priority;
    qint64 m_size;
    qint64 m_resumePosition;
    qint64 m_downloadedBytes;
    int m_progress;
    qint64 m_speed;
    QTime m_downloadTime;
    Transfers::Status m_status;
    QString m_statusInfo;
    mutable bool m_convertible;
    mutable bool m_checkedIfConvertible;
    bool m_convert;
    int m_row;
    int m_preferredConnections;
    int m_maxConnections;
    QTime m_captchaTime;
    QString m_captchaResponse;
#ifdef QML_USER_INTERFACE
    bool m_expanded;
#endif
    QList<Connection*> m_connections;
    QList<Transfer*> m_transfers;
    QStringList m_archivePasswords;
};

#endif // TRANSFER_H
