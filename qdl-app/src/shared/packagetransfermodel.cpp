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

#include "packagetransfermodel.h"
#include "transfermodel.h"

PackageTransferModel::PackageTransferModel(QObject *parent, Transfer *package) :
    QAbstractListModel(parent)
{
#if (QT_VERSION >= 0x040600) && (QT_VERSION < 0x050000)
    this->setRoleNames(TransferModel::instance()->roleNames());
#endif
    if (package) {
        this->setPackage(package);
    }
}

PackageTransferModel::~PackageTransferModel() {}

Transfer* PackageTransferModel::package() const {
    return m_list.isEmpty() ? 0 : m_list.first();
}

void PackageTransferModel::setPackage(Transfer *package) {
    if (!package) {
        return;
    }

    this->beginRemoveRows(QModelIndex(), 0, this->rowCount() - 1);

    foreach (Transfer *transfer, m_list) {
#ifndef QML_USER_INTERFACE
        this->disconnect(transfer, SIGNAL(dataChanged(int)), this, SLOT(onTransferDataChanged(int)));
#endif
        this->disconnect(transfer, SIGNAL(statusChanged(Transfers::Status)), this, SLOT(onTransferStatusChanged(Transfers::Status)));
    }

    m_list.clear();

    this->endRemoveRows();
    this->beginInsertRows(QModelIndex(), 0, package->count());

    m_list.append(package);
    m_list.append(package->childTransfers());

    foreach (Transfer *transfer, m_list) {
#ifndef QML_USER_INTERFACE
        this->connect(transfer, SIGNAL(dataChanged(int)), this, SLOT(onTransferDataChanged(int)));
#endif
        this->connect(transfer, SIGNAL(statusChanged(Transfers::Status)), this, SLOT(onTransferStatusChanged(Transfers::Status)));
    }

    this->endInsertRows();

    emit countChanged(this->rowCount());
}

#if (QT_VERSION >= 0x050000) || (QT_VERSION < 0x040600)
QHash<int, QByteArray> PackageTransferModel::roleNames() const {
    return TransferModel::instance()->roleNames();
}
#endif

Qt::ItemFlags PackageTransferModel::flags(const QModelIndex &index) const {
    Q_UNUSED(index)

    return Qt::ItemIsEnabled;
}

int PackageTransferModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return m_list.size();
}

QVariant PackageTransferModel::data(const QModelIndex &index, int role) const {
    if (index.isValid()) {
        if (Transfer *transfer = this->get(index)) {
            return transfer->data(role);
        }
    }

    return QVariant();
}

QVariant PackageTransferModel::data(int row, const QByteArray &role) const {
    return this->data(this->index(row), this->roleNames().key(role));
}

QVariant PackageTransferModel::data(const QString &id, const QByteArray &role) const {
    if (Transfer *transfer = this->get(id)) {
        return transfer->data(this->roleNames().key(role));
    }

    return QVariant();
}

QMap<int, QVariant> PackageTransferModel::itemData(const QModelIndex &index) const {
    if (index.isValid()) {
        if (Transfer *transfer = this->get(index)) {
            return transfer->itemData();
        }
    }

    return QMap<int, QVariant>();
}

QVariantMap PackageTransferModel::itemData(int row) const {
    if (Transfer *transfer = this->get(row)) {
        return transfer->itemDataWithRoleNames();
    }

    return QVariantMap();
}

QVariantMap PackageTransferModel::itemData(const QString &id) const {
    foreach (Transfer *transfer, m_list) {
        if (transfer->id() == id) {
            return transfer->itemDataWithRoleNames();
        }
    }

    return QVariantMap();
}

QVariantList PackageTransferModel::allItemData() const {
    QVariantList list;

    foreach (Transfer *transfer, m_list) {
        list.append(transfer->itemDataWithRoleNames());
    }

    return list;
}

bool PackageTransferModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid()) {
        if (Transfer *transfer = this->get(index)) {
            return transfer->setData(role, value);
        }
    }

    return false;
}

bool PackageTransferModel::setData(int row, const QVariant &value, const QByteArray &role) {
    return this->setData(this->index(row), value, this->roleNames().key(role));
}

bool PackageTransferModel::setData(const QString &id, const QVariant &value, const QByteArray &role) {
    if (Transfer *transfer = this->get(id)) {
        return transfer->setData(this->roleNames().key(role), value);
    }

    return false;
}

Transfer* PackageTransferModel::get(const QModelIndex &index) const {
    if ((index.row() >= 0) && (index.row() < this->rowCount())) {
        return m_list.at(index.row());
    }

    return 0;
}

Transfer* PackageTransferModel::get(int row) const {
    return this->get(this->index(row));
}

Transfer* PackageTransferModel::get(const QString &id) const {
    foreach (Transfer *transfer, m_list) {
        if (transfer->id() == id) {
            return transfer;
        }
    }

    return 0;
}

#ifndef QML_USER_INTERFACE
void PackageTransferModel::onTransferDataChanged(int role) {
    Q_UNUSED(role)

    if (Transfer *transfer = qobject_cast<Transfer*>(this->sender())) {
        QModelIndex index = this->index(transfer->rowNumber());
        emit dataChanged(index, index);
    }
}
#endif

void PackageTransferModel::onTransferStatusChanged(Transfers::Status status) {
    switch (status) {
    case Transfers::Completed:
    case Transfers::Canceled:
        if (Transfer *transfer = qobject_cast<Transfer*>(this->sender())) {
            if ((transfer->rowNumber() >= 0) && (transfer->rowNumber() < this->rowCount())) {
                this->beginRemoveRows(QModelIndex(), transfer->rowNumber(), transfer->rowNumber());

                m_list.removeAt(transfer->rowNumber());

                this->endRemoveRows();
                emit countChanged(this->rowCount());
            }
        }

        return;
    default:
        return;
    }
}
