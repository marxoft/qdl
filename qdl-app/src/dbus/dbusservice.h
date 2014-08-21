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

#ifndef DBUSSERVICE_H
#define DBUSSERVICE_H

#include <QObject>
#include <QVariantList>
#include <QDBusVariant>

class DBusService : public QObject
{
    Q_OBJECT

public:
    ~DBusService();

    static DBusService* instance();

public slots:
    void addUrls(const QStringList &urls);
    void addUrls(const QStringList &urls, const QString &service);
    void importUrls(const QStringList &urls);
    void importUrls(const QStringList &urls, const QString &service);
    void retrieveUrls(const QStringList &urls);

    QVariantList getTransfers() const;
    QVariantMap getTransfer(const QString &id) const;

    QVariant getTransferProperty(const QString &id, const QString &property);
    bool setTransferProperty(const QString &id, const QString &property, const QDBusVariant &value);

    bool start();
    bool pause();
    bool start(const QString &id);
    bool pause(const QString &id);
    bool remove(const QString &id);

private:
    DBusService();

private:
    static DBusService *self;
};

#endif // DBUSSERVICE_H
