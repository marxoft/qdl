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

#include "archivepasswordsmodel.h"
#include "database.h"

ArchivePasswordsModel::ArchivePasswordsModel(QObject *parent) :
    QStringListModel(parent)
{
    this->setStringList(Database::instance()->getArchivePasswords());
}

ArchivePasswordsModel::~ArchivePasswordsModel() {}

void ArchivePasswordsModel::addPassword(const QString &password) {
    if (Database::instance()->addArchivePassword(password)) {
        this->setStringList(Database::instance()->getArchivePasswords());
    }
}

void ArchivePasswordsModel::removePassword(const QString &password) {
    if (Database::instance()->removeArchivePassword(password)) {
        this->setStringList(Database::instance()->getArchivePasswords());
    }
}

void ArchivePasswordsModel::removePassword(int row) {
    this->removePassword(this->data(this->index(row), Qt::DisplayRole).toString());
}
