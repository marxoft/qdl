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

#ifndef PLUGINSETTINGSTAB_H
#define PLUGINSETTINGSTAB_H

#include <QWidget>

class PluginSettingsModel;
class QListView;
class QStackedWidget;
class QModelIndex;

class PluginSettingsTab : public QWidget
{
    Q_OBJECT

public:
    explicit PluginSettingsTab(QWidget *parent = 0);
    ~PluginSettingsTab();

private slots:
    void onItemActivated(const QModelIndex &index);

private:
    PluginSettingsModel *m_model;
    QListView *m_view;
    QStackedWidget *m_stack;
};

#endif // PLUGINSETTINGSTAB_H
