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

#ifndef CATEGORIESMODEL_H
#define CATEGORIESMODEL_H

#include <QAbstractTableModel>

class CategoriesModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        PathRole
    };

public:
    explicit CategoriesModel(QObject *parent = 0);
    ~CategoriesModel();
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
#if QT_VERSION >= 0x040600
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
#endif
    Q_INVOKABLE void loadCategories();

public slots:
    void addCategory(const QString &name, const QString &path);
    void removeCategory(const QString &name);
    void removeCategory(int row);

signals:
    void countChanged(int count);

private:
    QList< QPair<QString, QString> > m_list;
#if QT_VERSION >= 0x040600
    QHash<int, QByteArray> m_roleNames;
#endif
};

#endif // CATEGORIESMODEL_H
