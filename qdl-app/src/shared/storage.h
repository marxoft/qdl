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
    static Storage* instance();

    QString directory() const;
    void setDirectory(const QString &directory);

    QString errorString() const;

public slots:
    bool storeTransfers(QList<Transfer*> transfers, bool deleteWhenStored);
    bool restoreTransfers();
    bool clearStoredTransfers();
    
private:
    Storage();
    ~Storage();

    void setErrorString(const QString &errorString);

signals:
    void transfersStored();
    void transfersRestored(QList<Transfer*> transfers);
    void error();

private:
    static Storage *self;

    QString m_directory;
    QString m_errorString;
};

#endif // STORAGE_H
