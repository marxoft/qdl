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

#ifndef CATEGORYSETTINGSTAB_H
#define CATEGORYSETTINGSTAB_H

#include <QWidget>

class CategoriesModel;
class QTreeView;
class QPushButton;
class QLineEdit;
class QMenu;

class CategorySettingsTab : public QWidget
{
    Q_OBJECT

public:
    explicit CategorySettingsTab(QWidget *parent = 0);

private slots:
    void showContextMenu(const QPoint &pos);
    void addCategory();
    void removeCategory();
    void editCategory();
    void showFolderDialog();
    void onCategoryEditChanged();
    
private:
    CategoriesModel *m_model;
    QTreeView *m_view;
    QMenu *m_contextMenu;
    QAction *m_editAction;
    QAction *m_removeAction;
    QPushButton *m_addButton;
    QLineEdit *m_nameEdit;
    QLineEdit *m_pathEdit;
    QPushButton *m_browseButton;
    QPushButton *m_doneButton;
};

#endif // CATEGORYSETTINGSTAB_H
