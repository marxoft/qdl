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

#ifndef ARCHIVEPASSWORDSMODEL_H
#define ARCHIVEPASSWORDSMODEL_H

#include <QStringListModel>

class ArchivePasswordsModel : public QStringListModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    explicit ArchivePasswordsModel(QObject *parent = 0);
    ~ArchivePasswordsModel();
    
public slots:
    void addPassword(const QString &password);
    void removePassword(const QString &password);
    void removePassword(int row);

signals:
    void countChanged(int count);
};

#endif // ARCHIVEPASSWORDSMODEL_H
