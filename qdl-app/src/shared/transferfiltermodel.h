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
 
#ifndef TRANSFERFILTERMODEL_H
#define TRANSFERFILTERMODEL_H

#include "transfermodel.h"
#include <QSortFilterProxyModel>

class TransferFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString searchQuery
               READ searchQuery
               WRITE setSearchQuery
               NOTIFY searchQueryChanged)
    Q_PROPERTY(Transfers::Status statusFilter
               READ statusFilter
               WRITE setStatusFilter
               NOTIFY statusFilterChanged)

public:
    TransferFilterModel(QObject *parent = 0);
    ~TransferFilterModel();
    
    QString searchQuery() const;
    Transfers::Status statusFilter() const;
    
    Q_INVOKABLE QVariant modelIndex(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE QVariant parentModelIndex(const QModelIndex &child) const;
    Q_INVOKABLE QVariant mapFromSourceModelIndex(const QModelIndex &sourceIndex) const;
    Q_INVOKABLE QVariant mapToSourceModelIndex(const QModelIndex &proxyIndex) const;

public slots:
    void setSearchQuery(const QString &query);
    void setStatusFilter(Transfers::Status status);
    void resetFilters();
    
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    
signals:
    void searchQueryChanged(const QString &query);
    void statusFilterChanged(Transfers::Status status);
    
private:
    QString m_searchQuery;
    Transfers::Status m_statusFilter;
};

#endif // TRANSFERFILTERMODEL_H
