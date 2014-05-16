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

#ifndef PACKAGETRANSFERMODEL_H
#define PACKAGETRANSFERMODEL_H

#include "transfer.h"
#include <QAbstractListModel>

class PackageTransferModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(Transfer* package
               READ package
               WRITE setPackage)
    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit PackageTransferModel(QObject *parent = 0, Transfer *package = 0);
    ~PackageTransferModel();

    Transfer* package() const;
    void setPackage(Transfer *package);
#if (QT_VERSION >= 0x050000) || (QT_VERSION < 0x040600)
    QHash<int, QByteArray> roleNames() const;
#endif
    Qt::ItemFlags flags(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
    Q_INVOKABLE QVariant data(const QString &id, const QByteArray &role) const;

    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    Q_INVOKABLE QVariantMap itemData(int row) const;
    Q_INVOKABLE QVariantMap itemData(const QString &id) const;
    Q_INVOKABLE QVariantList allItemData() const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Q_INVOKABLE bool setData(int row, const QVariant &value, const QByteArray &role);
    Q_INVOKABLE bool setData(const QString &id, const QVariant &value, const QByteArray &role);

    Transfer* get(const QModelIndex &index) const;
    Q_INVOKABLE Transfer* get(int row) const;
    Q_INVOKABLE Transfer* get(const QString &id) const;

private slots:
#ifndef QML_USER_INTERFACE
    void onTransferDataChanged(int role);
#endif
    void onTransferStatusChanged(Transfers::Status status);

signals:
    void countChanged(int count);

private:
    QList<Transfer*> m_list;
};

#endif // PACKAGETRANSFERMODEL_H
