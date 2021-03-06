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

#ifndef PLUGINSETTINGSMODEL_H
#define PLUGINSETTINGSMODEL_H

#include <QAbstractListModel>

typedef struct {
    QString name;
    QString icon;
    QString fileName;
} Setting;

class PluginSettingsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

public:
    enum Roles {
        PluginNameRole = Qt::UserRole + 1,
        PluginIconRole,
        FileNameRole
    };

public:
    explicit PluginSettingsModel(QObject *parent = 0);
    ~PluginSettingsModel();
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
#if QT_VERSION >= 0x040600
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
#endif

public slots:
    void loadSettingsFiles();

signals:
    void countChanged(int count);
    
private:
    QList<Setting> m_list;
#if QT_VERSION >= 0x040600
    QHash<int, QByteArray> m_roleNames;
#endif
};

#endif // PLUGINSETTINGSMODEL_H
