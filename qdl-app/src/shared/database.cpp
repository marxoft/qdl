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

#include "database.h"
#include "transfer.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QVariant>
#include <QDir>
#include <QDebug>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

Database* Database::self = 0;

Database::Database() :
    QObject()
{
    if (!self) {
        self = this;
    }
#if QT_VERSION >= 0x050000
    QString path(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.QDL/");
#else
    QString path(QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/.QDL/");
#endif
    QDir dir;
    m_database = QSqlDatabase::addDatabase("QSQLITE");

    if (dir.mkpath(path)) {
        m_database.setDatabaseName(path + "QDL.db");
    }
    else {
        m_database.setDatabaseName("QDL.db");
    }

    this->initialize();
}

Database::~Database() {}

Database* Database::instance() {
    return !self ? new Database : self;
}

void Database::initialize() {
    if (m_database.open()) {
        m_database.exec("CREATE TABLE IF NOT EXISTS categories (name TEXT UNIQUE, path TEXT)");
        m_database.exec("CREATE TABLE IF NOT EXISTS accounts (serviceName TEXT UNIQUE, username TEXT, password TEXT)");
        m_database.exec("CREATE TABLE IF NOT EXISTS archivePasswords (password TEXT UNIQUE)");
    }
    else {
        qDebug() << m_database.lastError().text();
    }
}

bool Database::addCategory(const QString &name, const QString &path) {
    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        query.prepare("INSERT OR REPLACE INTO categories VALUES (?, ?)");
        query.addBindValue(name);
        query.addBindValue((path.endsWith('/')) || (path.isEmpty()) ? path : path + '/');
        bool success = query.exec();
        m_database.close();

        if (success) {
            emit categoriesChanged();
        }

        return success;
    }

    return false;
}

bool Database::removeCategory(const QString &name) {
    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        query.prepare("DELETE FROM categories WHERE name = ?");
        query.addBindValue(name);
        bool success = query.exec();
        m_database.close();

        if (success) {
            emit categoriesChanged();
        }

        return success;
    }

    return false;
}

QString Database::getCategoryPath(const QString &name) {
    QString path;

    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        QSqlRecord record;
        query.prepare("SELECT path FROM categories WHERE name = ?");
        query.addBindValue(name);
        query.exec();
        record = query.record();

        if (record.count() > 0) {
            while (query.next()) {
                path = query.value(0).toString();
            }
        }

        m_database.close();
    }

    return path;
}

QList< QPair<QString, QString> > Database::getCategories() {
    QList< QPair<QString, QString> > categories;

    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        QSqlRecord record;
        query.exec("SELECT * FROM categories ORDER BY name ASC");
        record = query.record();

        if (record.count() > 0) {
            while (query.next()) {
                QPair<QString, QString> category;
                category.first = query.value(0).toString();
                category.second = query.value(1).toString();
                categories.append(category);
            }
        }

        m_database.close();
    }

    return categories;
}

QStringList Database::getCategoryNames() {
    QStringList names(tr("Default"));

    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        QSqlRecord record;
        query.exec("SELECT name FROM categories ORDER BY name ASC");
        record = query.record();

        if (record.count() > 0) {
            while (query.next()) {
                names.append(query.value(0).toString());
            }
        }

        m_database.close();
    }

    return names;
}

bool Database::addAccount(const QString &serviceName, const QString &username, const QString &password) {
    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        query.prepare("INSERT OR REPLACE INTO accounts VALUES (?, ?, ?)");
        query.addBindValue(serviceName);
        query.addBindValue(username);
        query.addBindValue(password);
        bool success = query.exec();
        m_database.close();

        return success;
    }

    return false;
}

bool Database::removeAccount(const QString &serviceName) {
    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        query.prepare("DELETE FROM accounts WHERE serviceName = ?");
        query.addBindValue(serviceName);
        bool success = query.exec();
        m_database.close();

        return success;
    }

    return false;
}

QPair<QString, QString> Database::getAccount(const QString &serviceName) {
    QPair<QString, QString> account;

    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        QSqlRecord record;
        query.prepare("SELECT username, password FROM accounts WHERE serviceName = ?");
        query.addBindValue(serviceName);
        query.exec();
        record = query.record();

        if (record.count() > 0) {
            while (query.next()) {
                account.first = query.value(0).toString();
                account.second = query.value(1).toString();
            }
        }

        m_database.close();
    }

    return account;
}

bool Database::addArchivePassword(const QString &password) {
    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        query.prepare("INSERT OR REPLACE INTO archivePasswords VALUES (?)");
        query.addBindValue(password);
        bool success = query.exec();
        m_database.close();

        return success;
    }

    return false;
}

bool Database::removeArchivePassword(const QString &password) {
    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        query.prepare("DELETE FROM archivePasswords WHERE password = ?");
        query.addBindValue(password);
        bool success = query.exec();
        m_database.close();

        return success;
    }

    return false;
}

QStringList Database::getArchivePasswords() {
    QStringList passwords;

    if ((m_database.isOpen()) || (m_database.open())) {
        QSqlQuery query;
        QSqlRecord record;
        query.exec("SELECT password FROM archivePasswords");
        record = query.record();

        if (record.count() > 0) {
            while (query.next()) {
                passwords.append(query.value(0).toString());
            }
        }

        m_database.close();
    }

    return passwords;
}
