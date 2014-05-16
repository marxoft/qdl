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

#include "categorysettingstab.h"
#include "newcategorydialog.h"
#include "../shared/categoriesmodel.h"
#include <QVBoxLayout>
#include <QTreeView>
#include <QMenu>
#include <QPushButton>

CategorySettingsTab::CategorySettingsTab(QWidget *parent) :
    QWidget(parent),
    m_model(new CategoriesModel(this)),
    m_view(new QTreeView(this)),
    m_contextMenu(new QMenu(this)),
    m_editAction(m_contextMenu->addAction(tr("Edit"), this, SLOT(editCategory()))),
    m_removeAction(m_contextMenu->addAction(tr("Remove"), this, SLOT(removeCategory()))),
    m_newButton(new QPushButton(tr("New"), this))
{
    m_view->setModel(m_model);
    m_view->setRootIsDecorated(false);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setExpandsOnDoubleClick(false);
    m_view->setItemsExpandable(false);
    m_view->setColumnWidth(0, 200);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(m_view);
    vbox->addWidget(m_newButton, 0, Qt::AlignRight);

    this->connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(editCategory()));
    this->connect(m_view, SIGNAL(activated(QModelIndex)), this, SLOT(editCategory()));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    this->connect(m_newButton, SIGNAL(clicked()), this, SLOT(addCategory()));
}

void CategorySettingsTab::showContextMenu(const QPoint &pos) {
    m_contextMenu->popup(m_view->mapToGlobal(pos), m_editAction);
}

void CategorySettingsTab::addCategory() {
    NewCategoryDialog *dialog = new NewCategoryDialog(this);
    dialog->open();
    this->connect(dialog, SIGNAL(addCategory(QString,QString)), m_model, SLOT(addCategory(QString,QString)));
}

void CategorySettingsTab::removeCategory() {
    if (m_view->currentIndex().isValid()) {
        m_model->removeCategory(m_view->currentIndex().row());
    }
}

void CategorySettingsTab::editCategory() {
    QModelIndex index = m_view->currentIndex();

    if (index.isValid()) {
        NewCategoryDialog *dialog = new NewCategoryDialog(this);
        dialog->setWindowTitle(tr("Edit category"));
        dialog->setName(index.data(CategoriesModel::NameRole).toString());
        dialog->setPath(index.data(CategoriesModel::PathRole).toString());
        dialog->open();
        this->connect(dialog, SIGNAL(addCategory(QString,QString)), m_model, SLOT(addCategory(QString,QString)));
    }
}
