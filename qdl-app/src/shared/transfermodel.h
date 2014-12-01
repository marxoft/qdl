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

#ifndef TRANSFERMODEL_H
#define TRANSFERMODEL_H

#include "enums.h"
#include <QAbstractItemModel>

class Transfer;
class QTimer;

class TransferModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)
    Q_PROPERTY(Transfers::Action nextAction
               READ nextAction
               WRITE setNextAction
               NOTIFY nextActionChanged)
    Q_PROPERTY(int totalDownloadSpeed
               READ totalDownloadSpeed
               NOTIFY totalDownloadSpeedChanged)
    Q_PROPERTY(int activeTransfers
               READ activeTransfers
               NOTIFY activeTransfersChanged)

public:
#if (QT_VERSION >= 0x050000) || (QT_VERSION < 0x040600)
    QHash<int, QByteArray> roleNames() const;
#endif

    Qt::DropActions supportedDropActions() const;
    
    Qt::ItemFlags flags(const QModelIndex &index) const;
    
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    
#ifdef TABLE_TRANSFER_VIEW
    QVariant headerData(int section, Qt::Orientation orientation = Qt::Horizontal, int role = Qt::DisplayRole) const;
#endif

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant data(int row, int parentRow, const QByteArray &role) const;
    Q_INVOKABLE QVariant data(const QString &id, const QByteArray &role) const;

    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    Q_INVOKABLE QVariantMap itemData(int row, int parentRow) const;
    Q_INVOKABLE QVariantMap itemData(const QString &id) const;
    Q_INVOKABLE QVariantList allItemData(Transfers::Status filter = Transfers::Unknown,
                                         const QString &query = QString(), int start = 0, int count = -1) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Q_INVOKABLE bool setData(int row, int parentRow, const QVariant &value, const QByteArray &role);
    Q_INVOKABLE bool setData(const QString &id, const QVariant &value, const QByteArray &role);

    Transfer* get(const QModelIndex &index) const;
    Q_INVOKABLE Transfer* get(int row, int parentRow) const;
    Q_INVOKABLE Transfer* get(const QString &id) const;
    
    void move(const QModelIndex &sourceParent, int sourceRow, const QModelIndex &destinationParent, int destinationRow);
    Q_INVOKABLE void move(int sourceParentRow, int sourceRow, int destinationParentRow, int destinationRow);

    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value,
                          int hits = -1, Qt::MatchFlags flags = Qt::MatchExactly) const;

    Transfers::Action nextAction() const;

    int totalDownloadSpeed() const;

    int activeTransfers() const;

    static TransferModel* instance();

public slots:
    void setNextAction(Transfers::Action action);

    void addTransfer(const QUrl &url, const QString &service, const QString &fileName);

    bool start();
    bool pause();
    bool start(const QString &id);
    bool pause(const QString &id);
    bool cancel(const QString &id);

    void storeTransfers();
    void restoreStoredTransfers();

private:
    TransferModel();
    ~TransferModel();

    void getNextTransfers();

    bool canAddActiveTransfer(Transfer *transfer);
    void addActiveTransfer(Transfer *transfer);
    void removeActiveTransfer(Transfer *transfer);

private slots:
    void startNextTransfers();
#ifdef QML_USER_INTERFACE
    void onTransferSpeedChanged();
#else
    void onTransferDataChanged(int role);
#endif
    void onTransferStatusChanged(Transfers::Status status);
    void onMaximumConcurrentTransfersChanged(int oldMaximum, int newMaximum);
    void storeAndDeleteTransfers();

signals:
    void countChanged(int count);
    void nextActionChanged(Transfers::Action action);
    void totalDownloadSpeedChanged(int speed);
    void activeTransfersChanged(int active);

private:
    static TransferModel *self;

    Transfer *m_rootItem;
    QTimer *m_queueTimer;
    QList<Transfer*> m_activeTransfers;
    Transfers::Action m_nextAction;
    QHash<int, QByteArray> m_roleNames;
};

#endif // TRANSFERMODEL_H
