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

#include "transferfiltermodel.h"
#include "transfermodel.h"
#include "transfer.h"

TransferFilterModel::TransferFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent),
    m_statusFilter(Transfers::Unknown)
{
    this->setFilterRole(Transfer::NameRole);
    this->setSortRole(Transfer::NameRole);
    this->setFilterCaseSensitivity(Qt::CaseInsensitive);
    this->setSortCaseSensitivity(Qt::CaseInsensitive);
    this->setDynamicSortFilter(true);
    this->setSourceModel(TransferModel::instance());
}

TransferFilterModel::~TransferFilterModel() {}

QString TransferFilterModel::searchQuery() const {
    return m_searchQuery;
}

void TransferFilterModel::setSearchQuery(const QString &query) {
    if (query != this->searchQuery()) {
        m_searchQuery = query;
        this->setFilterFixedString(query);
        emit searchQueryChanged(query);
    }
}

Transfers::Status TransferFilterModel::statusFilter() const {
    return m_statusFilter;
}

void TransferFilterModel::setStatusFilter(Transfers::Status status) {
    if (status != this->statusFilter()) {
        m_statusFilter = status;
        this->invalidateFilter();
        emit statusFilterChanged(status);
    }
}

void TransferFilterModel::resetFilters() {
    this->setSearchQuery(QString());
    this->setStatusFilter(Transfers::Unknown);
}

bool TransferFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    switch (this->statusFilter()) {
    case Transfers::Unknown:
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    default:
        return (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
                && (this->sourceModel()->data(this->sourceModel()->index(source_row, 0, source_parent), Transfer::StatusRole)
                    == this->statusFilter());
    }
}

