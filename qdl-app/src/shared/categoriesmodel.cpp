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

#include "categoriesmodel.h"
#include "database.h"

CategoriesModel::CategoriesModel(QObject *parent) :
    QAbstractTableModel(parent)
{
#if QT_VERSION >= 0x040600
    m_roleNames[NameRole] = "name";
    m_roleNames[PathRole] = "path";
#if QT_VERSION < 0x050000
    this->setRoleNames(m_roleNames);
#endif
#endif
    this->loadCategories();
}

CategoriesModel::~CategoriesModel() {}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> CategoriesModel::roleNames() const {
    return m_roleNames;
}
#endif

int CategoriesModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return m_list.size();
}

int CategoriesModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return 2;
}

QVariant CategoriesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return tr("Name");
    case 1:
        return tr("Download path");
    default:
        return QVariant();
    }
}

QVariant CategoriesModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case NameRole:
        return m_list.at(index.row()).first;
    case PathRole:
        return m_list.at(index.row()).second;
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return m_list.at(index.row()).first;
        case 1:
            return m_list.at(index.row()).second;
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}

#if QT_VERSION >= 0x040600
QVariant CategoriesModel::data(int row, const QByteArray &role) const {
    return this->data(this->index(row, 0), this->roleNames().key(role));
}
#endif

void CategoriesModel::loadCategories() {
    if (!m_list.isEmpty()) {
        this->beginRemoveRows(QModelIndex(), 0, this->rowCount() - 1);
        m_list.clear();
        this->endRemoveRows();
    }

    QList< QPair<QString, QString> > categories = Database::instance()->getCategories();

    this->beginInsertRows(QModelIndex(), 0, categories.size() - 1);
    m_list = categories;
    this->endInsertRows();
    
    emit countChanged(this->rowCount());
}

void CategoriesModel::addCategory(const QString &name, const QString &path) {
    if (Database::instance()->addCategory(name, path)) {
        this->loadCategories();
    }
}

void CategoriesModel::removeCategory(const QString &name) {
    if (Database::instance()->removeCategory(name)) {
        this->loadCategories();
    }
}

void CategoriesModel::removeCategory(int row) {
    this->removeCategory(this->data(this->index(row, 0), NameRole).toString());
}
