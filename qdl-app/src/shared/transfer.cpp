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

#include "transfer.h"
#include "utils.h"
#include "database.h"
#include "settings.h"
#include "networkaccessmanager.h"
#include "connection.h"
#include "pluginmanager.h"
#include "definitions.h"
#include "audioconverter.h"
#include "archiveextractor.h"
#ifndef QML_USER_INTERFACE
#include "../captchadialog/captchadialog.h"
#endif
#include <QPixmap>
#include <QDateTime>
#include <QNetworkReply>
#include <QDir>
#include <qplatformdefs.h>
#ifdef MEEGO_EDITION_HARMATTAN
#include "../harmattan/captchaimageprovider.h"
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#endif

static const qint64 MIN_FRAGMENT_SIZE = 1024 * 2048;
static const QString VIDEO_SUFFIXES("mp4:flv:avi:divx:mpg:mpeg:mpeg2:mpeg4:ts:mkv:wmv:xvid:mov");
static const QString ARCHIVE_SUFFIXES("rar:zip:tar:gz");
static const QRegExp ILLEGAL_CHARS_RE("[\"@&~=\\/:?#!|<>*^]");

Transfer::Transfer(QObject *parent) :
    QObject(parent),
    m_parent(0),
    m_servicePlugin(0),
    m_recaptchaPlugin(0),
    m_decaptchaPlugin(0),
    m_nam(0),
    m_converter(0),
    m_extractor(0),
    m_priority(Transfers::NormalPriority),
    m_size(0),
    m_resumePosition(0),
    m_downloadedBytes(0),
    m_progress(0),
    m_speed(0),
    m_status(Transfers::Paused),
    m_convertible(false),
    m_checkedIfConvertible(false),
    m_convert(false),
    m_row(0),
    m_preferredConnections(1),
    m_maxConnections(1)
  #ifdef QML_USER_INTERFACE
    ,m_expanded(false)
  #endif
{
    this->connect(&m_file, SIGNAL(writeCompleted()), this, SLOT(onFileWriteCompleted()));
    this->connect(&m_file, SIGNAL(error()), this, SLOT(onFileError()));
}

Transfer::~Transfer() {}

QVariant Transfer::data(int role) const {
    switch (role) {
    case NameRole:
        return this->fileName();
    case IconRole:
        return this->iconFileName();
    case ServiceNameRole:
        return this->serviceName();
    case CategoryRole:
        return this->category();
    case PriorityRole:
        return this->priority();
    case PriorityStringRole:
        return this->priorityString();
    case SizeRole:
        return this->size();
    case PositionRole:
        return this->position();
    case ProgressRole:
        return this->progress();
    case StatusRole:
        return this->status();
    case StatusStringRole:
        return this->statusString();
    case ConvertibleToAudioRole:
        return this->convertibleToAudio();
    case ConvertToAudioRole:
        return this->convertToAudio();
    case PreferredConnectionsRole:
        return this->preferredConnections();
    case MaximumConnectionsRole:
        return this->maximumConnections();
    case DownloadResumableRole:
        return this->downloadIsResumable();
    case TransferCountRole:
        return this->count();
    case IdRole:
        return this->id();
    case PackageIdRole:
        return this->packageId();
    case PackageNameRole:
        return this->packageName();
    case PackageStatusRole:
        return this->packageStatus();
    case RowNumberRole:
        return this->rowNumber();
#ifdef QML_USER_INTERFACE
    case ExpandedRole:
        return this->expanded();
#endif
    default:
        return QVariant();
    }
}

QMap<int, QVariant> Transfer::itemData() const {
    QMap<int, QVariant> map;
    map[NameRole] = this->fileName();
    map[ServiceNameRole] = this->serviceName();
    map[IconRole] = this->iconFileName();
    map[CategoryRole] = this->category();
    map[PriorityRole] = this->priority();
    map[PriorityStringRole] = Transfers::priorityString(this->priority());
    map[SizeRole] = this->size();
    map[PositionRole] = this->position();
    map[ProgressRole] = this->progress();
    map[StatusRole] = this->status();
    map[StatusStringRole] = this->statusString();
    map[ConvertibleToAudioRole] = this->convertibleToAudio();
    map[ConvertToAudioRole] = this->convertToAudio();
    map[PreferredConnectionsRole] = this->preferredConnections();
    map[MaximumConnectionsRole] = this->maximumConnections();
    map[IdRole] = this->id();
    map[PackageIdRole] = this->packageId();

    return map;
}

QVariantMap Transfer::itemDataWithRoleNames() const {
    QVariantMap map;
    map["name"] = this->fileName();
    map["serviceName"] = this->serviceName();
    map["icon"] = this->iconFileName();
    map["category"] = this->category();
    map["priority"] = this->priority();
    map["priorityString"] = Transfers::priorityString(this->priority());
    map["size"] = this->size();
    map["position"] = this->position();
    map["progress"] = this->progress();
    map["status"] = this->status();
    map["statusString"] = this->statusString();
    map["convertibleToAudio"] = this->convertibleToAudio();
    map["convertToAudio"] = this->convertToAudio();
    map["preferredConnections"] = this->preferredConnections();
    map["maximumConnections"] = this->maximumConnections();
    map["id"] = this->id();
    map["packageId"] = this->packageId();

    return map;
}

bool Transfer::setData(int role, const QVariant &value) {
    switch (role) {
    case CategoryRole:
        this->setCategory(value.toString());
        break;
    case PriorityRole:
        this->setPriority(static_cast<Transfers::Priority>(value.toInt()));
        break;
    case StatusRole:
        switch (value.toInt()) {
        case Transfers::Queued:
            this->queue();
            break;
        case Transfers::Paused:
            this->pause();
            break;
        case Transfers::Cancelled:
            this->cancel();
            break;
        }

        break;
    case ConvertToAudioRole:
        this->setConvertToAudio((this->convertibleToAudio()) && (value.toBool()));
        break;
    case PreferredConnectionsRole:
        this->setPreferredConnections(value.toInt());
        break;
    case PackageStatusRole:
        switch (value.toInt()) {
        case Transfers::Queued:
            this->queuePackage();
            break;
        case Transfers::Paused:
            this->pausePackage();
            break;
        case Transfers::Cancelled:
            this->cancelPackage();
            break;
        }

        break;
#ifdef QML_USER_INTERFACE
    case ExpandedRole:
        this->setExpanded(value.toBool());
        break;
#endif
    default:
        return false;
    }

    return true;
}

bool Transfer::match(int role, const QVariant &value) {
    switch (role) {
    case NameRole:
        return this->fileName().contains(value.toString(), Qt::CaseInsensitive);
    case CategoryRole:
        return this->category() == value.toString();
    case PriorityRole:
        return this->priority() == value.toInt();
    case StatusRole:
        switch (value.toInt()) {
        case Transfers::Unknown:
            return true;
        default:
            return this->status() == value.toInt();
        }
    case PackageNameRole:
        return this->packageName().contains(value.toString(), Qt::CaseInsensitive);
    case PackageStatusRole:
        switch (value.toInt()) {
        case Transfers::Unknown:
            return true;
        default:
            return this->packageStatus() == value.toInt();
        }
    default:
        return false;
    }
}

bool Transfer::isValid() const {
    return !this->id().isEmpty();
}

bool Transfer::isPackage() const {
    return (this->isValid()) && ((!this->parentTransfer()) || (!this->parentTransfer()->isValid()));
}

int Transfer::rowNumber() const {
    return m_row;
}

void Transfer::setRowNumber(int row) {
    m_row = row;
}

#ifdef QML_USER_INTERFACE
bool Transfer::expanded() const {
    return m_expanded;
}

void Transfer::setExpanded(bool expanded) {
    if (expanded != this->expanded()) {
        m_expanded = expanded;
        emit expandedChanged();
    }
}
#endif

Transfer* Transfer::parentTransfer() const {
    return m_parent;
}

void Transfer::setParentTransfer(Transfer *transfer) {
    m_parent = transfer;

    if ((transfer) && (transfer->isValid())) {
        this->setPackageId(transfer->packageId());
        this->setPackageName(transfer->packageName());
        this->setPackageSuffix(transfer->packageSuffix());
        this->setCategory(transfer->category());
        this->setPriority(transfer->priority());
        this->setDownloadPath(transfer->downloadPath());
    }
}

QString Transfer::packageId() const {
    return m_packageId;
}

void Transfer::setPackageId(const QString &id) {
    m_packageId = id;
}

QString Transfer::packageName() const {
    return m_packageName;
}

void Transfer::setPackageName(const QString &name) {
    m_packageName = name;
    m_packageName.replace(ILLEGAL_CHARS_RE, "_");
}

QString Transfer::packageSuffix() const {
    return m_packageSuffix;
}

void Transfer::setPackageSuffix(const QString &suffix) {
    m_packageSuffix = suffix;
}

Transfers::Status Transfer::packageStatus() const {
    return m_packageStatus;
}

void Transfer::setPackageStatus(Transfers::Status status) {
    if (status != this->packageStatus()) {
        m_packageStatus = status;
#ifndef QML_USER_INTERFACE
        emit dataChanged(PackageStatusRole);
#endif
    }
}

void Transfer::queuePackage() {
    for (int i = this->count() - 1; i >= 0; i--) {
        this->childTransfer(i)->queue();
    }

    this->queue();
}

void Transfer::pausePackage() {
    for (int i = this->count() - 1; i >= 0; i--) {
        this->childTransfer(i)->pause();
    }

    this->pause();
}

void Transfer::cancelPackage() {
    for (int i = this->count() - 1; i >= 0; i--) {
        this->childTransfer(i)->cancel();
    }

    this->cancel();
}

QString Transfer::id() const {
    return m_id;
}

void Transfer::setId(const QString &id) {
    m_id = id;
}

QUrl Transfer::url() const {
    return m_url;
}

void Transfer::setUrl(const QUrl &url) {
    m_url = url;
}

QString Transfer::fileName() const {
    return m_fileName;
}

void Transfer::setFileName(const QString &fileName) {
    if (fileName != this->fileName()) {
        m_fileName = fileName;
        m_fileName.replace(ILLEGAL_CHARS_RE, "_");
#ifndef QML_USER_INTERFACE
        emit dataChanged(NameRole);
#endif
    }
}

QString Transfer::fileSuffix() const {
    return this->fileName().mid(this->fileName().lastIndexOf('.') + 1).toLower();
}

QString Transfer::downloadPath() const {
    return m_downloadPath;
}

void Transfer::setDownloadPath(const QString &path) {
    m_downloadPath = path.endsWith('/') ? path : path + '/';
}

QString Transfer::serviceName() const {
    return m_serviceName;
}

void Transfer::setServiceName(const QString &name) {
    if (name != this->serviceName()) {
        m_serviceName = name;
#ifndef QML_USER_INTERFACE
        emit dataChanged(NameRole);
#endif

        if (name.isEmpty()) {
            this->setIconFileName(ICON_PATH + "qdl.png");
            this->setPreferredConnections(1);
            this->setMaximumConnections(1);
        }
        else {
            ServicePlugin *plugin = PluginManager::instance()->getServicePlugin(name);
            this->setIconFileName(ICON_PATH + (!plugin ? "qdl.png" : plugin->iconName()));

            if ((plugin) && (plugin->maximumConnections() <= 0)) {
                // Plugin allows unlimited connections
                this->setMaximumConnections(MAX_CONNECTIONS);
                this->setPreferredConnections(Settings::instance()->maximumConnectionsPerTransfer(), false);
            }
            else {
                this->setMaximumConnections(!plugin ? 1 : qMin(plugin->maximumConnections(), MAX_CONNECTIONS));
                this->setPreferredConnections(!plugin ? 1 : qMin(plugin->maximumConnections(), Settings::instance()->maximumConnectionsPerTransfer()), false);
            }
        }
    }
}

QString Transfer::iconFileName() const {
    return m_iconFileName;
}

void Transfer::setIconFileName(const QString &fileName) {
    if (fileName != this->iconFileName()) {
        m_iconFileName = fileName;
#ifndef QML_USER_INTERFACE
        emit dataChanged(IconRole);
#endif
    }
}

QPixmap Transfer::icon() const {
    return QPixmap(this->iconFileName());
}

QString Transfer::category() const {
    return m_category;
}

void Transfer::setCategory(const QString &category) {
    if (category != this->category()) {
        m_category = category;
#ifdef QML_USER_INTERFACE
        emit categoryChanged();
#else
        emit dataChanged(CategoryRole);
#endif
        foreach (Transfer *transfer, m_transfers) {
            transfer->setCategory(category);
        }
    }
}

Transfers::Priority Transfer::priority() const {
    return m_priority;
}

void Transfer::setPriority(Transfers::Priority priority) {
    if (priority != this->priority()) {
        m_priority = priority;
#ifdef QML_USER_INTERFACE
        emit priorityChanged();
#else
        emit dataChanged(PriorityRole);
#endif
        foreach (Transfer *transfer, m_transfers) {
            transfer->setPriority(priority);
        }
    }
}

QString Transfer::priorityString() const {
    return Transfers::priorityString(this->priority());
}

qint64 Transfer::size() const {
    return m_size;
}

void Transfer::setSize(qint64 size) {
    if ((size > 0) && (size != this->size())) {
        m_size = size;
#ifdef QML_USER_INTERFACE
        emit sizeChanged();
#else
        emit dataChanged(SizeRole);
#endif
    }
}

qint64 Transfer::resumePosition() const {
    return m_resumePosition;
}

void Transfer::setResumePosition(qint64 position) {
    if (position != this->resumePosition()) {
        m_resumePosition = position;
#ifdef QML_USER_INTERFACE
        emit positionChanged();
#else
        emit dataChanged(PositionRole);
#endif
        if ((this->position() > 0) && (this->size() > 0)) {
            this->setProgress(position * 100 / this->size());
        }
    }
}

qint64 Transfer::position() const {
    return this->resumePosition() + m_downloadedBytes;
}

int Transfer::progress() const {
    return m_progress;
}

void Transfer::setProgress(int progress) {
    if (progress != this->progress()) {
        m_progress = progress;
#ifdef QML_USER_INTERFACE
        emit progressChanged();
#else
        emit dataChanged(ProgressRole);
#endif
    }
}

int Transfer::speed() const {
    switch (this->status()) {
    case Transfers::Downloading:
        if ((m_downloadedBytes > 0) && (m_downloadTime.elapsed() > 0)) {
            return m_downloadedBytes / m_downloadTime.elapsed();
        }

        break;
    default:
        break;
    }

    return 0;
}

Transfers::Status Transfer::status() const {
    return m_status;
}

void Transfer::setStatus(Transfers::Status status) {
    if (status != this->status()) {
        m_status = status;

        switch (status) {
        case Transfers::Downloading:
            m_downloadTime.start();
            break;
        case Transfers::Cancelled:
            if (!m_file.remove()) {
                QFile::remove(QString("%1%2%3").arg(this->downloadPath()).arg(this->downloadPath().endsWith("/") ? "" : "/").arg(this->fileName()));
            }

            if ((m_transfers.isEmpty()) && (this->isPackage())) {
                QDir().rmdir(this->downloadPath());
            }

            if (m_servicePlugin) m_servicePlugin->cancelCurrentOperation();
            if (m_recaptchaPlugin) m_recaptchaPlugin->cancelCurrentOperation();
            if (m_decaptchaPlugin) m_decaptchaPlugin->cancelCurrentOperation();
            break;
        case Transfers::Paused:
            if (m_servicePlugin) m_servicePlugin->cancelCurrentOperation();
            if (m_recaptchaPlugin) m_recaptchaPlugin->cancelCurrentOperation();
            if (m_decaptchaPlugin) m_decaptchaPlugin->cancelCurrentOperation();
        default:
            break;
        }

#ifdef QML_USER_INTERFACE
        emit statusInfoChanged();
#else
        emit dataChanged(StatusRole);
#endif
        emit statusChanged(status);
    }
}

QString Transfer::statusString() const {
    switch (this->status()) {
    case Transfers::ShortWait:
    case Transfers::LongWait:
    case Transfers::Failed:
        return QString("%1 - %2").arg(Transfers::statusString(this->status())).arg(this->statusInfo());
    default:
        return Transfers::statusString(this->status());
    }
}

QString Transfer::statusInfo() const {
    return m_statusInfo;
}

void Transfer::setStatusInfo(const QString &info) {
    if (info != this->statusInfo()) {
        m_statusInfo = info;
#ifdef QML_USER_INTERFACE
        emit statusInfoChanged();
#else
        emit dataChanged(StatusRole);
#endif
    }
}

bool Transfer::convertibleToAudio() const {
    if ((!m_checkedIfConvertible) && (!this->fileName().isEmpty())) {
        m_convertible = (VIDEO_SUFFIXES.contains(this->fileSuffix())) && (QFile::exists("/usr/bin/ffmpeg"));
        m_checkedIfConvertible = true;
    }

    return m_convertible;
}

bool Transfer::convertToAudio() const {
    return m_convert;
}

void Transfer::setConvertToAudio(bool convert) {
    if (convert != this->convertToAudio()) {
        if ((!convert) || (this->convertibleToAudio())) {
            m_convert = convert;
#ifdef QML_USER_INTERFACE
            emit convertToAudioChanged();
#else
            emit dataChanged(ConvertToAudioRole);
#endif
        }
    }
}

int Transfer::preferredConnections() const {
    return m_preferredConnections;
}

void Transfer::setPreferredConnections(int pref, bool overrideGlobalSetting) {
    if ((pref != this->preferredConnections()) && (pref > 0) && (pref < this->maximumConnections())) {
        m_preferredConnections = pref;
#ifdef QML_USER_INTERFACE
        emit preferredConnectionsChanged();
#else
        emit dataChanged(PreferredConnectionsRole);
#endif
        if (overrideGlobalSetting) {
            this->disconnect(Settings::instance(), SIGNAL(maximumConnectionsPerTransferChanged(int,int)), this, SLOT(onMaximumConnectionsChanged(int,int)));
        }
        else {
            this->connect(Settings::instance(), SIGNAL(maximumConnectionsPerTransferChanged(int,int)), this, SLOT(onMaximumConnectionsChanged(int,int)));
        }

        if (this->status() == Transfers::Downloading) {
            if (pref > this->activeConnections()) {
                this->addConnections(pref - this->activeConnections());
            }
            else if (pref < this->activeConnections()) {
                this->removeConnections(this->activeConnections() - pref);
            }
        }
    }
}

int Transfer::maximumConnections() const {
    return m_maxConnections;
}

void Transfer::setMaximumConnections(int maximum) {
    if ((maximum != this->maximumConnections()) && (maximum <= MAX_CONNECTIONS)) {
        m_maxConnections = maximum;
    }
}

int Transfer::activeConnections() const {
    int active = 0;

    foreach (Connection *connection, m_connections) {
        if (connection->status() == Transfers::Downloading) {
            active += 1;
        }
    }

    return active;
}

bool Transfer::downloadIsResumable() const {
    return ((this->serviceName().isEmpty()) || (!m_servicePlugin) || (m_servicePlugin->downloadsAreResumable()));
}

QList<Connection*> Transfer::connections() const {
    return m_connections;
}

void Transfer::loadConnections() {
    if (m_connections.isEmpty()) {
        this->addConnection(0, 0);
    }
    else {
        MetaInfo info;
        info.path = this->downloadPath();
        info.name = this->fileName();
        info.size = this->size();
        info.bytesRemaining = this->size() - this->position();

        m_file.setMetaInfo(info);
        m_file.start(QThread::LowestPriority);

        this->addConnections(this->preferredConnections());
    }
}

void Transfer::restoreConnection(qint64 start, qint64 end) {
    this->addConnection(start, end, false);
}

void Transfer::addConnection(qint64 start, qint64 end, bool startWhenAdded) {
    if (!m_nam) {
        m_nam = NetworkAccessManager::create();
	    m_nam->setParent(this);
    }

    qDebug() << "Adding connection with range start:" << start << "end:" << end;

    Connection *connection = new Connection(m_nam, this);
    connection->setRequest(m_request);
    connection->setData(m_data);
    connection->setContentRange(start, end);
    m_connections.append(connection);
    this->connect(connection, SIGNAL(dataAvailable(qint64,QByteArray)), this, SLOT(onDataAvailable(qint64,QByteArray)));
    this->connect(connection, SIGNAL(bytesDownloaded(qint64)), this, SLOT(onBytesDownloaded(qint64)));
    this->connect(connection, SIGNAL(statusChanged(Transfers::Status)), this, SLOT(onConnectionStatusChanged(Transfers::Status)));

    if (m_connections.size() == 1) {
        this->connect(connection, SIGNAL(metaInfoReady(MetaInfo)), this, SLOT(onMetaInfoReady(MetaInfo)));
    }

    if (startWhenAdded) {
        connection->start();
    }
}

void Transfer::addConnections(int count) {
    qDebug() << "Adding" << count << "connections";

    if (count <= 0) {
        return;
    }

    int added = 0;
    int i = 0;

    while ((added < count) && (i < m_connections.size())) {
        if (m_connections.at(i)->status() != Transfers::Downloading) {
            m_connections.at(i)->setRequest(m_request);
            m_connections.at(i)->setData(m_data);
            m_connections.at(i)->start();
            added++;
        }

        i++;
    }

    i = 0;

    while ((added < count) && (i < m_connections.size())) {
        qint64 end = m_connections.at(i)->contentRangeEnd();

        if (end > 0) {
            qint64 pos = m_connections.at(i)->position();
            qint64 start = (end - ((end - pos) / 2));

            if ((end - start) > MIN_FRAGMENT_SIZE) {
                m_connections.at(i)->setContentRangeEnd(start);
                this->addConnection(start, end);
                added++;
            }
        }

        i++;
    }
}

void Transfer::removeConnections(int count) {
    qDebug() << "Removing" << count << "connections";

    if (count <= 0) {
        return;
    }

    int removed = 0;
    int i = 0;

    while ((removed < count) && (i < m_connections.size())) {
        if (m_connections.at(i)->status() == Transfers::Downloading) {
            m_connections.at(i)->pause();
            removed++;
        }

        i++;
    }
}

void Transfer::createPackage() {
    this->setPackageId(QString::number(Utils::currentMSecsSinceEpoch()));
    this->setPackageSuffix(this->fileName().mid(this->fileName().lastIndexOf('.') + 1).toLower());

    QString packageName = this->fileName().left(this->fileName().lastIndexOf(QRegExp("p(ar|)t\\d+\\.", Qt::CaseInsensitive)));
    packageName = packageName.left(packageName.lastIndexOf('.'));

    this->setPackageName(packageName);
}

bool Transfer::transferBelongsToPackage(Transfer *transfer) const {
    return ((transfer->fileName().startsWith(this->packageName()))
            && (transfer->fileName() != this->fileName())
            && (ARCHIVE_SUFFIXES.contains(transfer->fileSuffix()))
            && (transfer->fileSuffix() == this->packageSuffix()));
}

Transfer* Transfer::childTransfer(int row) const {
    if ((row >= 0) && (row < m_transfers.size())) {
        return m_transfers.at(row);
    }

    return 0;
}

QList<Transfer*> Transfer::childTransfers() const {
    return m_transfers;
}

int Transfer::count() const {
    return m_transfers.size();
}

void Transfer::addChildTransfer(Transfer *transfer) {
    m_transfers.append(transfer);
    transfer->setRowNumber(m_transfers.size() - 1);
    transfer->setParentTransfer(this);
#ifdef QML_USER_INTERFACE
    emit countChanged();
#else
    emit dataChanged(TransferCountRole);
#endif
}

void Transfer::insertChildTransfer(int row, Transfer *transfer) {
    m_transfers.insert(row, transfer);
    transfer->setRowNumber(row);
    transfer->setParentTransfer(this);

    for (int i = row + 1; i < m_transfers.size(); i++) {
        m_transfers.at(i)->setRowNumber(i);
    }
#ifdef QML_USER_INTERFACE
    emit countChanged();
#else
    emit dataChanged(TransferCountRole);
#endif
}

Transfer* Transfer::removeChildTransfer(int row) {
    if ((row >= 0) && (row < m_transfers.size())) {
        Transfer *transfer = m_transfers.takeAt(row);

        for (int i = row; i < m_transfers.size(); i++) {
            m_transfers.at(i)->setRowNumber(i);
        }
#ifdef QML_USER_INTERFACE
        emit countChanged();
#else
        emit dataChanged(TransferCountRole);
#endif
        return transfer;
    }

    return 0;
}

Transfer* Transfer::removeChildTransfer(Transfer *transfer) {
    return this->removeChildTransfer(m_transfers.indexOf(transfer));
}

void Transfer::queue() {
    switch (this->status()) {
    case Transfers::Paused:
    case Transfers::Failed:
        this->setStatus(Transfers::Queued);
        return;
    default:
        return;
    }
}

void Transfer::start() {
    switch(this->status()) {
    case Transfers::Paused:
    case Transfers::Failed:
    case Transfers::Queued:
    case Transfers::ShortWait:
	break;
    default:
	return;
    }

    if (!m_nam) {
        m_nam = NetworkAccessManager::create();
    }

    qint64 bytes = m_downloadedBytes;

    if (bytes > 0) {
        m_downloadedBytes = 0;
        this->setResumePosition(this->resumePosition() + bytes);
    }

    if (this->serviceName().isEmpty()) {
        this->onDownloadRequestReady(QNetworkRequest(this->url()));
        return;
    }

    if (!m_servicePlugin) {
        m_servicePlugin = PluginManager::instance()->createServicePlugin(this->serviceName());

        if (!m_servicePlugin) {
            this->setStatusInfo(tr("No service plugin found"));
            this->setStatus(Transfers::Failed);
            return;
        }

        m_servicePlugin->setParent(this);
        m_servicePlugin->setNetworkAccessManager(m_nam);

        this->connect(m_servicePlugin, SIGNAL(waiting(int)), this, SLOT(onServicePluginWaiting(int)));
        this->connect(m_servicePlugin, SIGNAL(statusChanged(ServicePlugin::Status)), this, SLOT(onServicePluginStatusChanged(ServicePlugin::Status)));
        this->connect(m_servicePlugin, SIGNAL(error(ServicePlugin::ErrorType)), this, SLOT(onServicePluginError(ServicePlugin::ErrorType)));
        this->connect(m_servicePlugin, SIGNAL(downloadRequestReady(QNetworkRequest,QByteArray)), this, SLOT(onDownloadRequestReady(QNetworkRequest,QByteArray)));
    }

    m_servicePlugin->getDownloadRequest(this->url());
}

void Transfer::pause() {
    switch (this->status()) {
    case Transfers::Extracting:
    case Transfers::Converting:
    case Transfers::Failed:
        return;
    case Transfers::Downloading:
        foreach (Connection *connection, m_connections) {
            connection->pause();
        }

        return;
    default:
        this->setStatus(Transfers::Paused);
        return;
    }
}

void Transfer::cancel() {
    switch (this->status()) {
    case Transfers::Extracting:
    case Transfers::Converting:
        return;
    case Transfers::Downloading:
        foreach (Connection *connection, m_connections) {
            connection->cancel();
        }

        return;
    default:
        this->setStatus(Transfers::Cancelled);
        return;
    }
}

void Transfer::onServicePluginWaiting(int msecs) {
    this->setStatusInfo(Utils::durationFromMSecs(msecs));
}

void Transfer::onServicePluginStatusChanged(ServicePlugin::Status status) {
    if (this->status() == Transfers::Paused) {
        return;
    }

    switch (status) {
    case ServicePlugin::Connecting:
        this->setStatus(Transfers::Connecting);
        return;
    case ServicePlugin::ShortWait:
        this->setStatus(Transfers::ShortWait);
        return;
    case ServicePlugin::LongWait:
        this->setStatus(Transfers::LongWait);
        return;
    case ServicePlugin::CaptchaRequired:
        this->onCaptchaRequired();
        return;
    case ServicePlugin::Ready:
	switch (this->status()) {
	case Transfers::ShortWait:
	    this->start();
	    return;
        default:
            this->setStatus(Transfers::Queued);
            return;
	}
    }
}

void Transfer::onServicePluginError(ServicePlugin::ErrorType errorType) {
    switch (errorType) {
    case ServicePlugin::UrlError:
        this->setStatusInfo(tr("Cannot retreive download url"));
        break;
    case ServicePlugin::CaptchaError:
        this->onCaptchaIncorrect();
        return;
    case ServicePlugin::Unauthorised:
        this->setStatusInfo(tr("Service authorisation error"));
        break;
    case ServicePlugin::BadRequest:
        this->setStatusInfo(tr("Bad request"));
        break;
    case ServicePlugin::NotFound:
        this->setStatusInfo(tr("File not available for download"));
        break;
    case ServicePlugin::TrafficExceeded:
        this->setStatusInfo(tr("Maximum download traffic exceeded"));
        break;
    case ServicePlugin::ServiceUnavailable:
        this->setStatusInfo(tr("Service unavailable"));
        break;
    case ServicePlugin::NetworkError:
        this->setStatusInfo(tr("Network error"));
        break;
    default:
        this->setStatusInfo(m_servicePlugin->errorString().isEmpty() ? tr("Cannot retreive download url") : m_servicePlugin->errorString());
    }

    this->setStatus(Transfers::Failed);
}

void Transfer::onCaptchaRequired() {
    if ((!m_servicePlugin) || (m_servicePlugin->recaptchaKey().isEmpty())) {
        this->setStatusInfo(tr("Cannot retrieve captcha due to service plugin error"));
        this->setStatus(Transfers::Failed);
        return;
    }

    if (!m_recaptchaPlugin) {
        m_recaptchaPlugin = PluginManager::instance()->createRecaptchaPlugin(m_servicePlugin->recaptchaServiceName());

        if (!m_recaptchaPlugin) {
            this->setStatusInfo(tr("No recaptcha plugin found"));
            this->setStatus(Transfers::Failed);
            return;
        }

        m_recaptchaPlugin->setParent(this);
        m_recaptchaPlugin->setNetworkAccessManager(m_nam);

        this->connect(m_recaptchaPlugin, SIGNAL(error(RecaptchaPlugin::ErrorType)), this, SLOT(onRecaptchaPluginError(RecaptchaPlugin::ErrorType)));
        this->connect(m_recaptchaPlugin, SIGNAL(captchaReady(QByteArray)), this, SLOT(onCaptchaReady(QByteArray)));
    }
    else if (m_recaptchaPlugin->serviceName() != m_servicePlugin->recaptchaServiceName()) {
        delete m_recaptchaPlugin;
        m_recaptchaPlugin = PluginManager::instance()->createRecaptchaPlugin(m_servicePlugin->recaptchaServiceName());

        if (!m_recaptchaPlugin) {
            this->setStatusInfo(tr("No recaptcha plugin found"));
            this->setStatus(Transfers::Failed);
            return;
        }

        m_recaptchaPlugin->setParent(this);
        m_recaptchaPlugin->setNetworkAccessManager(m_nam);

        this->connect(m_recaptchaPlugin, SIGNAL(error(RecaptchaPlugin::ErrorType)), this, SLOT(onRecaptchaPluginError(RecaptchaPlugin::ErrorType)));
        this->connect(m_recaptchaPlugin, SIGNAL(captchaReady(QByteArray)), this, SLOT(onCaptchaReady(QByteArray)));
    }

    this->setStatusInfo(tr("Retrieving captcha challenge"));

    m_recaptchaPlugin->getCaptcha(m_servicePlugin->recaptchaKey());
}

void Transfer::onRecaptchaPluginError(RecaptchaPlugin::ErrorType errorType) {
    switch (errorType) {
    case RecaptchaPlugin::NetworkError:
        this->setStatusInfo(tr("Network error"));
        break;
    case RecaptchaPlugin::Unauthorised:
        this->setStatusInfo(tr("Invalid captcha key"));
        break;
    case RecaptchaPlugin::ServiceUnavailable:
        this->setStatusInfo(tr("Recaptcha service unavailable"));
        break;
    case RecaptchaPlugin::InternalError:
        this->setStatusInfo(tr("Internal server error at recaptcha service"));
        break;
    default:
        this->setStatusInfo(tr("Cannot retrieve captcha challenge"));
    }

    this->setStatus(Transfers::Failed);
}

void Transfer::onCaptchaReady(const QByteArray &imageData) {
    if (!Settings::instance()->decaptchaService().isEmpty()) {
        if (!m_decaptchaPlugin) {
            m_decaptchaPlugin = PluginManager::instance()->createDecaptchaPlugin(Settings::instance()->decaptchaService());

            if (!m_decaptchaPlugin) {
                this->setStatusInfo(tr("No decaptcha plugin found"));
                this->setStatus(Transfers::Failed);
                return;
            }

            m_decaptchaPlugin->setParent(this);
            m_decaptchaPlugin->setNetworkAccessManager(m_nam);

            this->connect(m_decaptchaPlugin, SIGNAL(error(DecaptchaPlugin::ErrorType)), this, SLOT(onDecaptchaPluginError(DecaptchaPlugin::ErrorType)));
            this->connect(m_decaptchaPlugin, SIGNAL(captchaResponseReady(QString)), this, SLOT(onCaptchaResponseReady(QString)));
        }
        else if (Settings::instance()->decaptchaService() != m_decaptchaPlugin->serviceName()) {
            delete m_decaptchaPlugin;
            m_decaptchaPlugin = PluginManager::instance()->createDecaptchaPlugin(Settings::instance()->decaptchaService());

            if (!m_decaptchaPlugin) {
                this->setStatusInfo(tr("No decaptcha plugin found"));
                this->setStatus(Transfers::Failed);
                return;
            }

            m_decaptchaPlugin->setParent(this);
            m_decaptchaPlugin->setNetworkAccessManager(m_nam);

            this->connect(m_decaptchaPlugin, SIGNAL(error(DecaptchaPlugin::ErrorType)), this, SLOT(onDecaptchaPluginError(DecaptchaPlugin::ErrorType)));
            this->connect(m_decaptchaPlugin, SIGNAL(captchaResponseReady(QString)), this, SLOT(onCaptchaResponseReady(QString)));
        }

        this->setStatusInfo(QString("%1 %2").arg(tr("Retrieving captcha response from")).arg(m_decaptchaPlugin->serviceName()));

        m_decaptchaPlugin->getCaptchaResponse(imageData);
    }
    else {
#ifdef MEEGO_EDITION_HARMATTAN
        this->setStatusInfo(tr("Awaiting captcha response"));
        QDeclarativeView *view = new QDeclarativeView;
        view->engine()->addImageProvider(QString("captcha"), new CaptchaImageProvider);
        view->rootContext()->setContextProperty("Utils", new Utils(view));
        view->setSource(QUrl("qrc:/CaptchaDialog.qml"));

        if (QGraphicsObject *dialog = view->rootObject()) {
            dialog->setProperty("captchaImage", QString("image://captcha/" + imageData.toBase64()));
            dialog->setProperty("timeOut", 120);
            this->connect(dialog, SIGNAL(rejected()), view, SLOT(deleteLater()));
            this->connect(dialog, SIGNAL(rejected()), this, SLOT(onCaptchaRejectedByUser()));
            this->connect(dialog, SIGNAL(captchaResponseReady(QString)), view, SLOT(deleteLater()));
            this->connect(dialog, SIGNAL(captchaResponseReady(QString)), this, SLOT(onCaptchaResponseReady(QString)));
            view->showFullScreen();
        }
        else {
            this->setStatusInfo(tr("Cannot load captcha dialog"));
            this->setStatus(Transfers::Failed);
        }
#else
        QPixmap image;

        if (image.loadFromData(imageData)) {
            this->setStatusInfo(tr("Awaiting captcha response"));
            CaptchaDialog *dialog = new CaptchaDialog;
            dialog->setCaptchaImage(image);
            dialog->setTimeout(120);
            dialog->open();
            this->connect(dialog, SIGNAL(captchaResponseReady(QString)), this, SLOT(onCaptchaResponseReady(QString)));
            this->connect(dialog, SIGNAL(rejected()), this, SLOT(onCaptchaRejectedByUser()));
        }
        else {
            this->setStatusInfo(tr("Captcha image error"));
            this->setStatus(Transfers::Failed);
        }
#endif
    }
}

void Transfer::onCaptchaIncorrect() {
    if ((m_decaptchaPlugin) && (!m_decaptchaPlugin->captchaId().isEmpty())) {
        m_decaptchaPlugin->reportIncorrectCaptchaResponse(m_decaptchaPlugin->captchaId());
    }

    this->onCaptchaRequired();
}

void Transfer::onCaptchaRejectedByUser() {
    this->setStatusInfo(tr("No captcha response"));
    this->setStatus(Transfers::Failed);
}

void Transfer::onDecaptchaPluginError(DecaptchaPlugin::ErrorType errorType) {
    switch (errorType) {
    case DecaptchaPlugin::CaptchaNotFound:
        this->setStatusInfo(tr("Invalid captcha ID"));
        break;
    case DecaptchaPlugin::CaptchaUnsolved:
        this->onCaptchaIncorrect();
        return;
    case DecaptchaPlugin::ServiceUnavailable:
        this->setStatusInfo(tr("Decaptcha service unavailable"));
        break;
    case DecaptchaPlugin::Unauthorised:
        this->setStatusInfo(tr("Decaptcha service authorisation error"));
        break;
    case DecaptchaPlugin::BadRequest:
        this->setStatusInfo(tr("Decaptcha bad request"));
        break;
    case DecaptchaPlugin::InternalError:
        this->setStatusInfo(tr("Internal server error at decaptcha service"));
        break;
    case DecaptchaPlugin::NetworkError:
        this->setStatusInfo(tr("Network error"));
        break;
    default:
        this->setStatusInfo(tr("Decaptcha service error"));
    }

    this->setStatus(Transfers::Failed);
}

void Transfer::onCaptchaResponseReady(const QString &response) {
    if ((!m_servicePlugin) || (!m_recaptchaPlugin)) {
        this->setStatusInfo(tr("Cannot submit captcha response"));
        this->setStatus(Transfers::Failed);
        return;
    }

    this->setStatusInfo(tr("Submitting captcha response"));

    m_servicePlugin->submitCaptchaResponse(m_recaptchaPlugin->challenge(), response);
}

void Transfer::onDownloadRequestReady(const QNetworkRequest &request, const QByteArray &data) {
    m_request = request;
    m_data = data;
    this->loadConnections();
}

void Transfer::onMetaInfoReady(MetaInfo info) {
    this->setSize(info.size);

    if (info.name.size() > this->fileName().size()) {
        this->setFileName(info.name);
    }
    else {
        info.name = this->fileName();
    }

    info.path = this->downloadPath();
    info.name = this->fileName();
    m_file.setMetaInfo(info);
    m_file.start(QThread::LowestPriority);

    if (!m_connections.isEmpty()) {
        m_connections.first()->processData();
    }

    if (this->preferredConnections() > 1) {
        this->addConnections(this->preferredConnections() - this->activeConnections());
    }
}

void Transfer::onBytesDownloaded(qint64 bytes) {
    m_downloadedBytes += bytes;
#ifdef QML_USER_INTERFACE
    emit positionChanged();
    emit speedChanged();
#else
    emit dataChanged(PositionRole);
#endif
    if (this->size() > 0) {
        this->setProgress(this->position() * 100 / this->size());
    }
}

void Transfer::onDataAvailable(qint64 offset, const QByteArray &data) {
    m_file.write(offset, data);
}

void Transfer::onFileError() {
    foreach (Connection *connection, m_connections) {
        connection->pause();
    }

    this->setStatusInfo(m_file.errorString());
    this->setStatus(Transfers::Failed);
}

void Transfer::onConnectionStatusChanged(Transfers::Status status) {
    switch (status) {
    case Transfers::Downloading:
        break;
    case Transfers::Completed:
        if (Connection* connection = qobject_cast<Connection*>(this->sender())) {
            this->onConnectionCompleted(connection);
            return;
        }

        break;
    case Transfers::Failed:
        if (this->activeConnections() > 0) {
            return;
        }

        if (Connection* connection = qobject_cast<Connection*>(this->sender())) {
            this->setStatusInfo(connection->errorString());
        }

        break;
    case Transfers::Cancelled:
        foreach (Connection *connection, m_connections) {
            if (connection->status() != status) {
                return;
            }
        }

        break;
    default:
        foreach (Connection *connection, m_connections) {
            if (connection->status() != status) {
                return;
            }
        }

        break;
    }

    this->setStatus(status);
}

void Transfer::onConnectionCompleted(Connection *connection) {
    if (connection) {
        m_connections.removeOne(connection);
        connection->deleteLater();

        if (m_connections.isEmpty()) {
            if (this->size() <= 0) {
                this->onFileWriteCompleted();
            }
        }
        else if ((this->activeConnections() < this->preferredConnections()) && ((this->size() - this->position()) > MIN_FRAGMENT_SIZE)) {
            this->addConnections();
        }
    }
}

void Transfer::onFileWriteCompleted() {
    if ((m_transfers.isEmpty()) && (this->isPackage())) {
        if (this->convertToAudio()) {
            this->startAudioConversion();
        }
        else if ((ARCHIVE_SUFFIXES.contains(this->fileSuffix())) && (Settings::instance()->extractDownloadedArchives())) {
            m_archivePasswords = Database::instance()->getArchivePasswords();
            this->extractArchive();
        }
        else {
            this->moveFiles();
        }
    }
    else {
        this->setStatus(Transfers::Completed);
    }
}

void Transfer::onMaximumConnectionsChanged(int oldMaximum, int newMaximum) {
    Q_UNUSED(oldMaximum)

    ServicePlugin *plugin = PluginManager::instance()->getServicePlugin(this->serviceName());
    this->setPreferredConnections(!plugin ? 1 : qMin(plugin->maximumConnections(), newMaximum));
}

void Transfer::moveFiles() {
    QDir destinationDir(Settings::instance()->downloadPath());

    if (!this->category().isEmpty()) {
        QString categoryPath = Database::instance()->getCategoryPath(this->category());

        if (!categoryPath.isEmpty()) {
            destinationDir.setPath(categoryPath);
        }
    }

    if (!destinationDir.mkpath(destinationDir.path())) {
        destinationDir.setPath(Settings::instance()->downloadPath());

        if (!destinationDir.mkpath(destinationDir.path())) {
            this->setStatusInfo(tr("Cannot move downloaded files"));
            this->setStatus(Transfers::Failed);
            return;
        }
    }

    QDir downloadDir(this->downloadPath());

    foreach (QString oldFileName, downloadDir.entryList(QDir::Files)) {
        QString fileName = destinationDir.path() + "/" + oldFileName;

        int i = 1;

        while ((destinationDir.exists(fileName)) && (i < 100)) {
            fileName = (i == 1 ? QString("%1(%2)%3").arg(fileName.left(fileName.lastIndexOf('.'))).arg(i).arg(fileName.mid(fileName.lastIndexOf('.')))
                              : QString("%1(%2)%3").arg(fileName.left(fileName.lastIndexOf('('))).arg(i).arg(fileName.mid(fileName.lastIndexOf('.'))));
            i++;
        }

        qDebug() << "Renaming downloaded file to:" << fileName;

        if (!destinationDir.rename(downloadDir.absoluteFilePath(oldFileName), fileName)) {
            this->setStatusInfo(tr("Cannot move downloaded files"));
            this->setStatus(Transfers::Failed);
            return;
        }
    }

    downloadDir.rmdir(downloadDir.path());

    this->setStatus(Transfers::Completed);
}

void Transfer::startAudioConversion() {
    this->setStatus(Transfers::Converting);

    if (!m_converter) {
        m_converter = new AudioConverter(this);
        this->connect(m_converter, SIGNAL(finished()), this, SLOT(onAudioConversionFinished()));
        this->connect(m_converter, SIGNAL(error()), this, SLOT(onAudioConversionError()));
    }

    m_converter->start(this->downloadPath() + this->fileName(), this->downloadPath());
}

void Transfer::onAudioConversionFinished() {
    m_file.remove();
    this->setFileName(this->fileName().left(this->fileName().lastIndexOf('.')) + ".m4a");
    this->moveFiles();
}

void Transfer::onAudioConversionError() {
    this->moveFiles();
}

void Transfer::extractArchive(const QString &password) {
    this->setStatus(Transfers::Extracting);

    if (!m_extractor) {
        m_extractor = new ArchiveExtractor(this);
        this->connect(m_extractor, SIGNAL(finished()), this, SLOT(onArchiveExtractionFinished()));
        this->connect(m_extractor, SIGNAL(error()), this, SLOT(onArchiveExtractionError()));
    }

    QString outputDirectory = Database::instance()->getCategoryPath(this->category());

    if (outputDirectory.isEmpty()) {
        outputDirectory = Settings::instance()->downloadPath();
    }

    m_extractor->start(this->downloadPath() + this->fileName(), outputDirectory, password, Settings::instance()->createSubfolderForArchives());
}

void Transfer::onArchiveExtractionFinished() {
    if (Settings::instance()->deleteExtractedArchives()) {
        QDir dir(this->downloadPath());

        foreach (QString fileName, dir.entryList(QStringList() << "*." + this->fileSuffix())) {
            dir.remove(dir.absoluteFilePath(fileName));
        }

        dir.rmdir(this->downloadPath());

        this->setStatus(Transfers::Completed);
    }
    else {
        this->moveFiles();
    }
}

void Transfer::onArchiveExtractionError() {
    if (!m_archivePasswords.isEmpty()) {
        this->extractArchive(m_archivePasswords.takeFirst());
    }
    else {
        this->moveFiles();
    }
}
