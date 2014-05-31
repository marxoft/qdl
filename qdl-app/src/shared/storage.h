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

#ifndef STORAGE_H
#define STORAGE_H

#include "transfer.h"
#include <QObject>

class Storage : public QObject
{
    Q_OBJECT

public:
    explicit Storage(QObject *parent = 0);
    ~Storage();

    Q_INVOKABLE static bool storeTransfers(QList<Transfer*> transfers, bool deleteWhenStored);
    Q_INVOKABLE static QList<Transfer*> restoreTransfers();
    Q_INVOKABLE static bool clearStoredTransfers();
};

#endif // STORAGE_H
