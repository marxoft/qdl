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

#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QPair>
#include <QVariantMap>
#include <QStringList>

class Database : public QObject
{
    Q_OBJECT

public:
    static Database* instance();

public slots:
    bool addCategory(const QString &name, const QString &path);
    bool removeCategory(const QString &name);
    QString getCategoryPath(const QString &name);
    QList< QPair<QString, QString> > getCategories();
    QStringList getCategoryNames();

    bool addAccount(const QString &serviceName, const QString &username, const QString &password);
    bool removeAccount(const QString &serviceName);
    QPair<QString, QString> getAccount(const QString &serviceName);

    bool addArchivePassword(const QString &password);
    bool removeArchivePassword(const QString &password);
    QStringList getArchivePasswords();

private:
    Database();
    ~Database();

    void initialize();

signals:
    void error(const QString &message);
    void categoriesChanged();

private:
    static Database *self;

    QSqlDatabase m_database;
};

#endif // DATABASE_H
