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

#ifndef SELECTIONMODELS_H
#define SELECTIONMODELS_H

#include "enums.h"
#include <QStandardItemModel>

class SelectionModel : public QStandardItemModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit SelectionModel(QObject *parent = 0);
    ~SelectionModel();
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif
#if QT_VERSION >= 0x040600
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
#endif
    QString name(int row) const;
    QVariant value(int row) const;

    Q_INVOKABLE void addItem(const QString &name, const QVariant &value);

    Q_INVOKABLE void clear();

signals:
    void countChanged(int count);

#if QT_VERSION >= 0x040600
private:
    QHash<int, QByteArray> m_roleNames;
#endif
};

class ScreenOrientationModel : public SelectionModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit ScreenOrientationModel(QObject *parent = 0);

signals:
    void countChanged(int count);
};

class TransferPriorityModel : public SelectionModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit TransferPriorityModel(QObject *parent = 0);

signals:
    void countChanged(int count);
};

class LanguageModel : public SelectionModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit LanguageModel(QObject *parent = 0);

signals:
    void countChanged(int count);
};

class ConcurrentTransfersModel : public SelectionModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit ConcurrentTransfersModel(QObject *parent = 0);

signals:
    void countChanged(int count);
};

class ConnectionsModel : public SelectionModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit ConnectionsModel(QObject *parent = 0);

signals:
    void countChanged(int count);
};

class DownloadRateLimitModel : public SelectionModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit DownloadRateLimitModel(QObject *parent = 0);

signals:
    void countChanged(int count);
};

class StatusFilterModel : public SelectionModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit StatusFilterModel(QObject *parent = 0);

signals:
    void countChanged(int count);
};

class TransferActionModel : public SelectionModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit TransferActionModel(QObject *parent = 0);

signals:
    void countChanged(int count);
};

class NetworkProxyTypeModel : public SelectionModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit NetworkProxyTypeModel(QObject *parent = 0);

signals:
    void countChanged(int count);
};

#endif // SELECTIONMODELS_H
