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

#ifndef SERVICEACCOUNTSDIALOG_H
#define SERVICEACCOUNTSDIALOG_H

#include <QDialog>

class ServiceAccountsModel;
class QTreeView;
class QMenu;
class QModelIndex;

class ServiceAccountsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServiceAccountsDialog(QWidget *parent = 0);
    
private slots:
    void showContextMenu(const QPoint &pos);
    void removeAccount();
    void onItemClicked(const QModelIndex &index);

private:
    ServiceAccountsModel *m_model;
    QTreeView *m_view;
    QMenu *m_contextMenu;
};

#endif // SERVICEACCOUNTSDIALOG_H
