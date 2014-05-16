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

#include "decaptchaaccountsmodel.h"
#include "pluginmanager.h"
#include "database.h"
#include "../interfaces/decaptchaplugin.h"
#include "definitions.h"
#include <QPixmap>

DecaptchaAccountsModel::DecaptchaAccountsModel(QObject *parent) :
    QAbstractTableModel(parent)
{
#if QT_VERSION >= 0x040600
    m_roleNames[ServiceNameRole] = "serviceName";
    m_roleNames[ServiceIconRole] = "serviceIcon";
    m_roleNames[UsernameRole] = "username";
    m_roleNames[PasswordRole] = "password";
#if QT_VERSION < 0x050000
    this->setRoleNames(m_roleNames);
#endif
#endif
    this->loadAccounts();
}

DecaptchaAccountsModel::~DecaptchaAccountsModel() {}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> DecaptchaAccountsModel::roleNames() const {
    return m_roleNames;
}
#endif

int DecaptchaAccountsModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return m_list.size();
}

int DecaptchaAccountsModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return 3;
}

QVariant DecaptchaAccountsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return tr("Service");
    case 1:
        return QString("%1/%2").arg(tr("Username")).arg(tr("email"));
    case 2:
        return tr("Password");
    default:
        return QVariant();
    }
}

QVariant DecaptchaAccountsModel::data(const QModelIndex &index, int role) const {
    switch (role) {
    case ServiceNameRole:
        return m_list.at(index.row()).serviceName;
    case ServiceIconRole:
        return m_list.at(index.row()).serviceIcon;
    case UsernameRole:
        return m_list.at(index.row()).username;
    case PasswordRole:
        return m_list.at(index.row()).password;
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return m_list.at(index.row()).serviceName;
        case 1:
            return m_list.at(index.row()).username;
        case 2:
            return m_list.at(index.row()).password;
        default:
            return QVariant();
        }
    case Qt::DecorationRole:
        switch (index.column()) {
        case 0:
            return QPixmap(m_list.at(index.row()).serviceIcon).scaled(ICON_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}

#if QT_VERSION >= 0x040600
QVariant DecaptchaAccountsModel::data(int row, const QByteArray &role) const {
    return this->data(this->index(row, 0), this->roleNames().key(role));
}
#endif

void DecaptchaAccountsModel::loadAccounts() {
    this->beginRemoveRows(QModelIndex(), 0, this->rowCount() - 1);
    m_list.clear();
    this->endRemoveRows();

    for (int i = 0; i < PluginManager::instance()->decaptchaPlugins().size(); i++) {
        DecaptchaPlugin *plugin = PluginManager::instance()->decaptchaPlugins().at(i);

        if (plugin) {
            QPair<QString, QString> account = Database::instance()->getAccount(plugin->serviceName());
            DecaptchaAccount decaptchaAccount;
            decaptchaAccount.serviceName = plugin->serviceName();
            decaptchaAccount.serviceIcon = ICON_PATH + plugin->iconName();
            decaptchaAccount.username = account.first;
            decaptchaAccount.password = account.second;

            this->beginInsertRows(QModelIndex(), this->rowCount(), this->rowCount());
            m_list.append(decaptchaAccount);
            this->endInsertRows();
        }
    }

    emit countChanged(this->rowCount());
}

void DecaptchaAccountsModel::addAccount(const QString &serviceName, const QString &username, const QString &password) {
    if (Database::instance()->addAccount(serviceName, username, password)) {
        this->loadAccounts();
    }
}

void DecaptchaAccountsModel::removeAccount(const QString &serviceName) {
    if (Database::instance()->removeAccount(serviceName)) {
        this->loadAccounts();
    }
}

void DecaptchaAccountsModel::removeAccount(int row) {
    this->removeAccount(this->data(this->index(row, 0), ServiceNameRole).toString());
}
