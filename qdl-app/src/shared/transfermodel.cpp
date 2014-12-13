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

#include "transfermodel.h"
#include "transfer.h"
#include "storage.h"
#include "settings.h"
#include "urlchecker.h"
#include "pluginmanager.h"
#include "utils.h"
#include <QTimer>
#include <QCoreApplication>
#include <QMimeData>
#ifdef TABLE_TRANSFER_VIEW
#include <QIcon>
#endif

TransferModel* TransferModel::self = 0;

TransferModel::TransferModel() :
    QAbstractItemModel(),
    m_rootItem(new Transfer(this)),
    m_queueTimer(new QTimer(this)),
    m_nextAction(Transfers::Continue)
{
    if (!self) {
        self = this;
    }

    m_roleNames[Transfer::NameRole] = "name";
    m_roleNames[Transfer::ServiceNameRole] = "serviceName";
    m_roleNames[Transfer::IconRole] = "icon";
    m_roleNames[Transfer::CategoryRole] = "category";
    m_roleNames[Transfer::PriorityRole] = "priority";
    m_roleNames[Transfer::PriorityStringRole] = "priorityString";
    m_roleNames[Transfer::SizeRole] = "size";
    m_roleNames[Transfer::PositionRole] = "position";
    m_roleNames[Transfer::ProgressRole] = "progress";
    m_roleNames[Transfer::StatusRole] = "status";
    m_roleNames[Transfer::StatusStringRole] = "statusString";
    m_roleNames[Transfer::ConvertibleToAudioRole] = "convertibleToAudio";
    m_roleNames[Transfer::ConvertToAudioRole] = "convertToAudio";
    m_roleNames[Transfer::PreferredConnectionsRole] = "preferredConnections";
    m_roleNames[Transfer::MaximumConnectionsRole] = "maximumConnections";
    m_roleNames[Transfer::CaptchaFileNameRole] = "captchaFileName";
    m_roleNames[Transfer::CaptchaTimeOutRole] = "captchaTimeOut";
    m_roleNames[Transfer::CaptchaResponseRole] = "captchaResponse";
    m_roleNames[Transfer::DownloadResumableRole] = "downloadResumable";
    m_roleNames[Transfer::TransferCountRole] = "transferCount";
    m_roleNames[Transfer::IdRole] = "id";
    m_roleNames[Transfer::PackageIdRole] = "packageId";
    m_roleNames[Transfer::PackageNameRole] = "packageName";
    m_roleNames[Transfer::PackageStatusRole] = "packageStatus";
    m_roleNames[Transfer::RowNumberRole] = "rowNumber";
#ifdef QML_USER_INTERFACE
    m_roleNames[Transfer::ExpandedRole] = "expanded";
#endif
#if (QT_VERSION >= 0x040600) && (QT_VERSION < 0x050000)
    this->setRoleNames(m_roleNames);
#endif
    m_queueTimer->setSingleShot(true);
    m_queueTimer->setInterval(1000);

    this->connect(m_queueTimer, SIGNAL(timeout()), this, SLOT(startNextTransfers()));
    this->connect(UrlChecker::instance(), SIGNAL(urlReady(QUrl,QString,QString)),
                  this, SLOT(addTransfer(QUrl,QString,QString)));
    this->connect(Settings::instance(), SIGNAL(maximumConcurrentTransfersChanged(int,int)),
                  this, SLOT(onMaximumConcurrentTransfersChanged(int,int)));
    this->connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                  this, SLOT(storeAndDeleteTransfers()));
}

TransferModel::~TransferModel() {}

TransferModel* TransferModel::instance() {
    return !self ? new TransferModel : self;
}

#if (QT_VERSION >= 0x050000) || (QT_VERSION < 0x040600)
QHash<int, QByteArray> TransferModel::roleNames() const {
    return m_roleNames;
}
#endif

Qt::DropActions TransferModel::supportedDropActions() const {
    return Qt::MoveAction;
}

Qt::ItemFlags TransferModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::ItemIsDropEnabled;
    }

    if (!index.parent().isValid()) {
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

QStringList TransferModel::mimeTypes() const {
    return QStringList() << "application/x-qdltransfermodeldatalist";
}

QMimeData* TransferModel::mimeData(const QModelIndexList &indexes) const {
    if ((indexes.count() <= 0) || (this->mimeTypes().isEmpty())) {
        return 0;
    }
    
    QMimeData *data = new QMimeData();
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    QModelIndexList::ConstIterator it = indexes.begin();
    
    for (; it != indexes.end(); it++) {
        if ((*it).column() == 0) {
            stream << (*it).row() << ((*it).parent().isValid() ? (*it).parent().row() : -1);
        }
    }
    
    data->setData(this->mimeTypes().at(0), encoded);
    return data;
}

bool TransferModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const QModelIndex &parent) {
    
    Q_UNUSED(column)                             
                                 
    if ((!data) || (action != Qt::MoveAction) || (this->mimeTypes().isEmpty())) {
        return false;
    }
    
    QString format = this->mimeTypes().at(0);
    
    if (!data->hasFormat(format)) {
        return false;
    }
    
    if ((row < 0) || (row > this->rowCount(parent))) {
        row = this->rowCount(parent);
    }
    
    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    QList< QPair<int, int> > rows;
    
    while (!stream.atEnd()) {
        int r;
        int pr;
        stream >> r >> pr;
        rows.append(QPair<int, int>(r, pr));
    }
    
    for (int i = 0; i < rows.size(); i++) {
        const int r = rows.at(i).first;
        const int pr = rows.at(i).second;
        this->move(pr == -1 ? QModelIndex() : this->index(pr, 0), r, parent, row);
    }
    
    return true;
}

int TransferModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0) {
        return 0;
    }

    if (parent.isValid()) {
        if (Transfer *parentTransfer = this->get(parent)) {
            return parentTransfer->count();
        }
    }

    return m_rootItem->count();
}

int TransferModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
#ifdef TABLE_TRANSFER_VIEW
    return 6;
#else
    return 1;
#endif
}

QModelIndex TransferModel::index(int row, int column, const QModelIndex &parent) const {
    if (!this->hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    Transfer *parentTransfer;

    if (!parent.isValid()) {
        parentTransfer = m_rootItem;
    }
    else {
        parentTransfer = static_cast<Transfer*>(parent.internalPointer());
    }

    if (!parentTransfer) {
        return QModelIndex();
    }

    Transfer *transfer = parentTransfer->childTransfer(row);

    if (transfer) {
        return this->createIndex(row, column, transfer);
    }

    return QModelIndex();
}

QVariant TransferModel::modelIndex(int row, int column, const QModelIndex &parent) const {
    return QVariant::fromValue(this->index(row, column, parent));
}

QModelIndex TransferModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }

    Transfer *transfer = static_cast<Transfer*>(child.internalPointer());

    if (!transfer) {
        return QModelIndex();
    }

    Transfer *parentTransfer = transfer->parentTransfer();

    if ((!parentTransfer) || (parentTransfer == m_rootItem)) {
        return QModelIndex();
    }

    return this->createIndex(parentTransfer->rowNumber(), 0, parentTransfer);
}

QVariant TransferModel::parentModelIndex(const QModelIndex &child) const {
    return QVariant::fromValue(this->parent(child));
}

#ifdef TABLE_TRANSFER_VIEW
QVariant TransferModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
        return QVariant();
    }
    
    switch (section) {
    case 0:
        return tr("Name");
    case 1:
        return tr("Category");
    case 2:
        return tr("Connections");
    case 3:
        return tr("Priority");
    case 4:
        return tr("Progress");
    case 5:
        return tr("Status");
    default:
        return QVariant();
    }
}
#endif

QVariant TransferModel::data(const QModelIndex &index, int role) const {
    if (index.isValid()) {
        if (Transfer *transfer = this->get(index)) {
#ifdef TABLE_TRANSFER_VIEW
            switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                case 0:
                    return transfer->data(Transfer::NameRole);
                case 1:
                    return transfer->data(Transfer::CategoryRole);
                case 2:
                    return transfer->data(Transfer::PreferredConnectionsRole);
                case 3:
                    return transfer->data(Transfer::PriorityStringRole);
                case 4:
                    return transfer->data(Transfer::ProgressRole);
                case 5:
                    return transfer->data(Transfer::StatusStringRole);
                default:
                    return QVariant();
                }
                
                break;
            case Qt::DecorationRole:
                switch (index.column()) {
                case 0:
                    return QIcon(transfer->data(Transfer::IconRole).toString());
                default:
                    return QVariant();
                }
                
                break;
            default:
                break;
            }
#endif
            return transfer->data(role);
        }
    }

    return QVariant();
}

QVariant TransferModel::data(const QModelIndex &index, const QByteArray &role) const {
    return this->data(index, this->roleNames().key(role));
}

QVariant TransferModel::data(const QString &id, const QByteArray &role) const {
    if (Transfer *transfer = this->get(id)) {
        return transfer->data(this->roleNames().key(role));
    }

    return QVariant();
}

QMap<int, QVariant> TransferModel::itemData(const QModelIndex &index) const {
    if (index.isValid()) {
        if (Transfer *transfer = this->get(index)) {
            return transfer->itemData();
        }
    }

    return QMap<int, QVariant>();
}

QVariantMap TransferModel::itemData(const QString &id) const {
    foreach (Transfer *transfer, m_rootItem->childTransfers()) {
        if (transfer->id() == id) {
            return transfer->itemDataWithRoleNames();
        }

        foreach (Transfer *childTransfer, transfer->childTransfers()) {
            if (childTransfer->id() == id) {
                return childTransfer->itemDataWithRoleNames();
            }
        }
    }

    return QVariantMap();
}

QVariantList TransferModel::allItemData(Transfers::Status filter, const QString &query, int start, int count) const {
    QVariantList list;

    if ((start < 0) || (start > m_rootItem->count())) {
        start = 0;
    }

    count = start + count;

    if ((count <= 0) || (count > m_rootItem->count())) {
        count = m_rootItem->count();
    }

    int matches = 0;

    switch (filter) {
    case Transfers::Unknown:
        if (!query.isEmpty()) {
            foreach (Transfer *transfer, m_rootItem->childTransfers()) {
                if (transfer->match(Transfer::NameRole, query)) {
                    matches++;

                    if (matches > start) {
                        list.append(transfer->itemDataWithRoleNames());

                        foreach (Transfer *childTransfer, transfer->childTransfers()) {
                            list.append(childTransfer->itemDataWithRoleNames());
                        }
                    }

                    if (matches >= (start + count)) {
                        break;
                    }
                }
            }

            return list;
        }

        break;
    default:
        if (!query.isEmpty()) {
            foreach (Transfer *transfer, m_rootItem->childTransfers()) {
                if ((transfer->match(Transfer::StatusRole, filter)) && (transfer->match(Transfer::NameRole, query))) {
                    matches++;

                    if (matches > start) {
                        list.append(transfer->itemDataWithRoleNames());

                        foreach (Transfer *childTransfer, transfer->childTransfers()) {
                            list.append(childTransfer->itemDataWithRoleNames());
                        }
                    }

                    if (matches >= (start + count)) {
                        break;
                    }
                }
            }

            return list;
        }

        foreach (Transfer *transfer, m_rootItem->childTransfers()) {
            if (transfer->match(Transfer::StatusRole, filter)) {
                matches++;

                if (matches > start) {
                    list.append(transfer->itemDataWithRoleNames());

                    foreach (Transfer *childTransfer, transfer->childTransfers()) {
                        list.append(childTransfer->itemDataWithRoleNames());
                    }
                }

                if (matches >= (start + count)) {
                    break;
                }
            }
        }

        return list;
    };

    for (int i = start; i < count; i++) {
        list.append(m_rootItem->childTransfer(i)->itemDataWithRoleNames());

        foreach (Transfer *childTransfer, m_rootItem->childTransfer(i)->childTransfers()) {
            list.append(childTransfer->itemDataWithRoleNames());
        }
    }

    return list;
}

bool TransferModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid()) {
        if (Transfer *transfer = this->get(index)) {
            return transfer->setData(role, value);
        }
    }

    return false;
}

bool TransferModel::setData(const QModelIndex &index, const QVariant &value, const QByteArray &role) {
    return this->setData(index, value, this->roleNames().key(role));
}

bool TransferModel::setData(const QString &id, const QVariant &value, const QByteArray &role) {
    if (Transfer *transfer = this->get(id)) {
        return transfer->setData(this->roleNames().key(role), value);
    }

    return false;
}

Transfer* TransferModel::get(const QModelIndex &index) const {
    return index.isValid() ? static_cast<Transfer*>(index.internalPointer()) : m_rootItem;
}

Transfer* TransferModel::get(const QString &id) const {
    foreach (Transfer *transfer, m_rootItem->childTransfers()) {
        if (transfer->id() == id) {
            return transfer;
        }

        foreach (Transfer *childTransfer, transfer->childTransfers()) {
            if (childTransfer->id() == id) {
                return childTransfer;
            }
        }
    }

    return 0;
}

void TransferModel::move(const QModelIndex &sourceParent, int sourceRow,
                         const QModelIndex &destinationParent, int destinationRow) {
             
    Transfer *parentTransfer = this->get(sourceParent);
    
    if (parentTransfer) {
        this->beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationRow);
        Transfer *transfer = parentTransfer->removeChildTransfer(sourceRow);
        
        if (transfer) {
            parentTransfer = this->get(destinationParent);
            
            if (parentTransfer) {
                parentTransfer->insertChildTransfer(destinationRow, transfer);
            }
        }
        
        this->endMoveRows();
    }
}

QModelIndexList TransferModel::match(const QModelIndex &start, int role, const QVariant &value,
                                     int hits, Qt::MatchFlags flags) const {
                                     
    Q_UNUSED(flags)

    QModelIndexList matches;

    if (!start.parent().isValid()) {
        int i = start.row();

        while ((i < m_rootItem->count()) && ((hits == -1) || (matches.size() <= hits))) {
            if (m_rootItem->childTransfer(i)->match(role, value)) {
                matches.append(this->index(i, 0));
            }

            i++;
        }
    }

    return matches;
}

Transfers::Action TransferModel::nextAction() const {
    return m_nextAction;
}

void TransferModel::setNextAction(Transfers::Action action) {
    if (action != this->nextAction()) {
        m_nextAction = action;
        emit nextActionChanged(action);
        
        if ((action == Transfers::Continue)
            && (m_activeTransfers.size() < Settings::instance()->maximumConcurrentTransfers())) {
            
            this->getNextTransfers();
        }
    }
}

int TransferModel::totalDownloadSpeed() const {
    int speed = 0;

    foreach (Transfer *transfer, m_activeTransfers) {
        speed += transfer->speed();
    }

    return speed;
}

int TransferModel::activeTransfers() const {
    return m_activeTransfers.size();
}

void TransferModel::addTransfer(const QUrl &url, const QString &service, const QString &fileName) {
    Transfer *transfer = new Transfer;
    transfer->setId(QString::number(Utils::currentMSecsSinceEpoch()));
    transfer->setDownloadPath(Settings::instance()->downloadPath() + ".incomplete/" + transfer->id());
    transfer->setFileName(fileName);
    transfer->setUrl(url);
    transfer->setServiceName(service);
    transfer->setCategory(Settings::instance()->defaultCategory());

    int i = this->rowCount() - 1;
    bool packageFound = false;

    while ((i >= 0) && (!packageFound)) {
        if (Transfer *parentTransfer = this->get(this->index(i, 0))) {
            packageFound = parentTransfer->transferBelongsToPackage(transfer);

            if (packageFound) {
                const int count = parentTransfer->count();
                this->beginInsertRows(this->index(i, 0), count, count);
                parentTransfer->addChildTransfer(transfer);
                this->endInsertRows();
            }
        }

        i--;
    }

    if (!packageFound) {
        this->beginInsertRows(QModelIndex(), this->rowCount(), this->rowCount());
        transfer->createPackage();
        m_rootItem->addChildTransfer(transfer);
        this->endInsertRows();

        emit countChanged(this->rowCount());
    }
#ifdef QML_USER_INTERFACE
    this->connect(transfer, SIGNAL(speedChanged()), this, SLOT(onTransferSpeedChanged()));
#else
    this->connect(transfer, SIGNAL(dataChanged(int)), this, SLOT(onTransferDataChanged(int)));
#endif
    this->connect(transfer, SIGNAL(statusChanged(Transfers::Status)), this, SLOT(onTransferStatusChanged(Transfers::Status)));

    if ((Settings::instance()->startTransfersAutomatically())
        && (m_activeTransfers.size() < Settings::instance()->maximumConcurrentTransfers())) {
        this->addActiveTransfer(transfer);
        transfer->start();
    }
}

void TransferModel::getNextTransfers() {
    int priority = Transfers::HighPriority;

    while (priority <= Transfers::LowPriority) {
        foreach (Transfer *parentTransfer, m_rootItem->childTransfers()) {
            if ((parentTransfer->priority() == priority) && (parentTransfer->status() == Transfers::Queued)) {
                if (m_activeTransfers.size() < Settings::instance()->maximumConcurrentTransfers()) {
                    this->addActiveTransfer(parentTransfer);
                }
                else {
                    return;
                }
            }

            foreach (Transfer *transfer, parentTransfer->childTransfers()) {
                if ((transfer->priority() == priority) && (transfer->status() == Transfers::Queued)) {
                    if (m_activeTransfers.size() < Settings::instance()->maximumConcurrentTransfers()) {
                        this->addActiveTransfer(parentTransfer);
                    }
                    else {
                        return;
                    }
                }
            }
        }

        priority++;
    }
}

void TransferModel::startNextTransfers() {
    this->getNextTransfers();

    foreach (Transfer *transfer, m_activeTransfers) {
        transfer->start();
    }
}

void TransferModel::addActiveTransfer(Transfer *transfer) {
    m_activeTransfers.append(transfer);
    emit activeTransfersChanged(this->activeTransfers());
}

void TransferModel::removeActiveTransfer(Transfer *transfer) {
    m_activeTransfers.removeOne(transfer);
    emit activeTransfersChanged(this->activeTransfers());
}

bool TransferModel::start() {
    if (!m_rootItem->count()) {
        return false;
    }

    foreach (Transfer *transfer, m_rootItem->childTransfers()) {
        transfer->queuePackage();
    }

    return true;
}

bool TransferModel::pause() {
    if (!m_rootItem->count()) {
        return false;
    }

    foreach (Transfer *transfer, m_rootItem->childTransfers()) {
        transfer->pausePackage();
    }

    return true;
}

bool TransferModel::start(const QString &id) {
    if (Transfer *transfer = this->get(id)) {
        transfer->queue();
        return true;
    }

    return false;
}

bool TransferModel::pause(const QString &id) {
    if (Transfer *transfer = this->get(id)) {
        transfer->pause();
        return true;
    }

    return false;
}

bool TransferModel::cancel(const QString &id) {
    if (Transfer *transfer = this->get(id)) {
        transfer->cancel();
        return true;
    }

    return false;
}

#ifdef QML_USER_INTERFACE
void TransferModel::onTransferSpeedChanged() {
    emit totalDownloadSpeedChanged(this->totalDownloadSpeed());
}
#else
void TransferModel::onTransferDataChanged(int role) {
#ifdef TABLE_TRANSFER_VIEW
    if (Transfer *transfer = qobject_cast<Transfer*>(this->sender())) {
        if (Transfer *parentTransfer = transfer->parentTransfer()) {
            QModelIndex index;
            int column = 0;
            
            switch (role) {
            case Transfer::ProgressRole:
            case Transfer::PositionRole:
            case Transfer::SizeRole:
                column = 4;
                break;
            case Transfer::StatusRole:
                column = 5;
                break;
            case Transfer::CategoryRole:
                column = 1;
                break;
            case Transfer::PreferredConnectionsRole:
                column = 2;
                break;
            case Transfer::PriorityRole:
                column = 3;
                break;
            default:
                break;
            }

            if (parentTransfer == m_rootItem) {
                index = this->index(transfer->rowNumber(), column);
            }
            else {
                index = this->index(transfer->rowNumber(), column, this->index(parentTransfer->rowNumber(), column));
            }

            emit dataChanged(index, index);
            emit totalDownloadSpeedChanged(this->totalDownloadSpeed());
        }
    }
#else
    Q_UNUSED(role)

    if (Transfer *transfer = qobject_cast<Transfer*>(this->sender())) {
        if (Transfer *parentTransfer = transfer->parentTransfer()) {
            QModelIndex index;

            if (parentTransfer == m_rootItem) {
                index = this->index(transfer->rowNumber(), 0);
            }
            else {
                index = this->index(transfer->rowNumber(), 0, this->index(parentTransfer->rowNumber(), 0));
            }

            emit dataChanged(index, index);
            emit totalDownloadSpeedChanged(this->totalDownloadSpeed());
        }
    }
#endif
}
#endif

void TransferModel::onTransferStatusChanged(Transfers::Status status) {
    switch (status) {
    case Transfers::Queued:
        if (m_activeTransfers.size() < Settings::instance()->maximumConcurrentTransfers()) {
            m_queueTimer->start();
        }

        break;
    case Transfers::Paused:
    case Transfers::LongWait:
    {
        if (Transfer *transfer = qobject_cast<Transfer*>(this->sender())) {
            this->removeActiveTransfer(transfer);
        }

        if (m_activeTransfers.size() < Settings::instance()->maximumConcurrentTransfers()) {
            m_queueTimer->start();
        }

        break;
    }
    case Transfers::Failed:
    {
        if (Transfer *transfer = qobject_cast<Transfer*>(this->sender())) {
            this->removeActiveTransfer(transfer);
        }

        switch (this->nextAction()) {
        case Transfers::Pause:
            if (m_activeTransfers.isEmpty()) this->pause();
            break;
        case Transfers::Quit:
            if (m_activeTransfers.isEmpty()) QCoreApplication::instance()->quit();
            return;
        default:
            if (m_activeTransfers.size() < Settings::instance()->maximumConcurrentTransfers()) {
                m_queueTimer->start();
            }

            break;
        }
        
        QMetaObject::invokeMethod(this, "storeTransfers", Qt::QueuedConnection);
        break;
    }
    case Transfers::Canceled:
    case Transfers::Completed:
    {
        if (Transfer *transfer = qobject_cast<Transfer*>(this->sender())) {
            this->removeActiveTransfer(transfer);

            if (transfer->parentTransfer()) {
                if (transfer->parentTransfer() == m_rootItem) {
                    this->beginRemoveRows(QModelIndex(), transfer->rowNumber(), transfer->rowNumber());
                }
                else {
                    this->beginRemoveRows(this->index(transfer->parentTransfer()->rowNumber(), 0),
                                          transfer->rowNumber(), transfer->rowNumber());
                }

                transfer->parentTransfer()->removeChildTransfer(transfer->rowNumber());
                this->endRemoveRows();
            }

            if (transfer->count() > 0) {
                if (Transfer *firstChild = transfer->removeChildTransfer(0)) {
                    foreach (Transfer *childTransfer, transfer->childTransfers()) {
                        firstChild->addChildTransfer(childTransfer);
                    }

                    if (transfer->parentTransfer()) {
                        if (transfer->parentTransfer() == m_rootItem) {
                            this->beginInsertRows(QModelIndex(), transfer->rowNumber(), transfer->rowNumber());
                        }
                        else {
                            this->beginInsertRows(this->index(transfer->parentTransfer()->rowNumber(), 0),
                                                  transfer->rowNumber(), transfer->rowNumber());
                        }

                        transfer->parentTransfer()->insertChildTransfer(transfer->rowNumber(), firstChild);

                        this->endInsertRows();
                    }
                }
            }

            transfer->deleteLater();
            emit countChanged(this->rowCount());

            if (status == Transfers::Completed) {
                switch (this->nextAction()) {
                case Transfers::Pause:
                    if (m_activeTransfers.isEmpty()) this->pause();
                    break;
                case Transfers::Quit:
                    if (m_activeTransfers.isEmpty()) QCoreApplication::instance()->quit();
                    return;
                default:
                    if (m_activeTransfers.size() < Settings::instance()->maximumConcurrentTransfers()) {
                        m_queueTimer->start();
                    }

                    break;
                }
            }

            QMetaObject::invokeMethod(this, "storeTransfers", Qt::QueuedConnection);
        }
        
        break;
    }
    default:
        break;
    }

    emit totalDownloadSpeedChanged(this->totalDownloadSpeed());
}

void TransferModel::onMaximumConcurrentTransfersChanged(int oldMaximum, int newMaximum) {
    if (newMaximum > oldMaximum) {
        if (newMaximum > m_activeTransfers.size()) {
            this->startNextTransfers();
        }
    }
    else if (newMaximum < oldMaximum) {
        if (newMaximum < m_activeTransfers.size()) {
            const int diff = qMin(m_activeTransfers.size(), oldMaximum) - newMaximum;
            QSet<Transfer*> paused;
            
            for (int i = 0; i < diff; i++) {
                for (int p = Transfers::LowPriority; p >= Transfers::HighPriority; p--) {
                    for (int i = m_activeTransfers.size() - 1; i >= 0; i--) {
                        Transfer *transfer = m_activeTransfers.at(i);
                        
                        if ((transfer->priority() == p) && (!paused.contains(transfer))) {
                            transfer->pause();
                            paused.insert(transfer);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void TransferModel::storeTransfers() {
    if (m_rootItem->count()) {
        Storage::storeTransfers(m_rootItem->childTransfers(), false);
    }
    else {
        Storage::clearStoredTransfers();
    }
}

void TransferModel::storeAndDeleteTransfers() {
    Storage::storeTransfers(m_rootItem->childTransfers(), true);
}

void TransferModel::restoreStoredTransfers() {
    QList<Transfer*> transfers = Storage::restoreTransfers();

    if (transfers.isEmpty()) {
        return;
    }

    this->beginInsertRows(QModelIndex(), 0, transfers.size() - 1);

    foreach (Transfer *transfer, transfers) {
        m_rootItem->addChildTransfer(transfer);

#ifdef QML_USER_INTERFACE
        this->connect(transfer, SIGNAL(speedChanged()), this, SLOT(onTransferSpeedChanged()));
#else
        this->connect(transfer, SIGNAL(dataChanged(int)), this, SLOT(onTransferDataChanged(int)));
#endif
        this->connect(transfer, SIGNAL(statusChanged(Transfers::Status)),
                      this, SLOT(onTransferStatusChanged(Transfers::Status)));

        foreach (Transfer *childTransfer, transfer->childTransfers()) {
#ifdef QML_USER_INTERFACE
            this->connect(childTransfer, SIGNAL(speedChanged()), this, SLOT(onTransferSpeedChanged()));
#else
            this->connect(childTransfer, SIGNAL(dataChanged(int)), this, SLOT(onTransferDataChanged(int)));
#endif
            this->connect(childTransfer, SIGNAL(statusChanged(Transfers::Status)),
                          this, SLOT(onTransferStatusChanged(Transfers::Status)));
        }
    }

    this->endInsertRows();

    emit countChanged(this->rowCount());

    if (Settings::instance()->startTransfersAutomatically()) {
        this->start();
    }
}
