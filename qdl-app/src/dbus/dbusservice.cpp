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

#include "dbusservice.h"
#include "../shared/transfermodel.h"
#include "../shared/urlchecker.h"
#include "../shared/urlretriever.h"

DBusService* DBusService::self = 0;

DBusService::DBusService() :
    QObject()
{
    if (!self) {
        self = this;
    }
}

DBusService::~DBusService() {}

DBusService* DBusService::instance() {
    return !self ? new DBusService : self;
}

void DBusService::addUrls(const QStringList &urls) {
    UrlChecker::instance()->addUrlsToQueue(urls);
}

void DBusService::addUrls(const QStringList &urls, const QString &service) {
    UrlChecker::instance()->addUrlsToQueue(urls, service);
}

void DBusService::importUrls(const QStringList &urls) {
    foreach (QString url, urls) {
        UrlChecker::instance()->importUrlsFromTextFile(url);
    }
}

void DBusService::importUrls(const QStringList &urls, const QString &service) {
    foreach (QString url, urls) {
        UrlChecker::instance()->importUrlsFromTextFile(url, service);
    }
}

void DBusService::retrieveUrls(const QStringList &urls) {
    UrlRetriever::instance()->addUrlsToQueue(urls);
}

QVariantList DBusService::getTransfers() const {
    return TransferModel::instance()->allItemData();
}

QVariantMap DBusService::getTransfer(const QString &id) const {
    return TransferModel::instance()->itemData(id);
}

QVariant DBusService::getTransferProperty(const QString &id, const QString &property) {
    return TransferModel::instance()->data(id, property.toUtf8());
}

bool DBusService::setTransferProperty(const QString &id, const QString &property, const QDBusVariant &value) {
    return TransferModel::instance()->setData(id, value.variant(), property.toUtf8());
}

bool DBusService::start() {
    return TransferModel::instance()->start();
}

bool DBusService::pause() {
    return TransferModel::instance()->pause();
}

bool DBusService::start(const QString &id) {
    return TransferModel::instance()->start(id);
}

bool DBusService::pause(const QString &id) {
    return TransferModel::instance()->pause(id);
}

bool DBusService::remove(const QString &id) {
    return TransferModel::instance()->cancel(id);
}
