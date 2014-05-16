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

#include "categoriesdialog.h"
#include "newcategorydialog.h"
#include "../shared/categoriesmodel.h"
#include <QTreeView>
#include <QMenu>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGridLayout>

CategoriesDialog::CategoriesDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new CategoriesModel(this)),
    m_view(new QTreeView(this)),
    m_contextMenu(new QMenu(this))
{
    this->setWindowTitle(tr("Categories"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setFixedHeight(340);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, this);
    QPushButton *addButton = buttonBox->addButton(tr("New"), QDialogButtonBox::ActionRole);
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_view, 0, 0);
    grid->addWidget(buttonBox, 0, 1);

    m_view->setModel(m_model);
    m_view->setRootIsDecorated(false);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setExpandsOnDoubleClick(false);
    m_view->setItemsExpandable(false);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setColumnWidth(0, 200);

    m_contextMenu->addAction(tr("Remove"), this, SLOT(removeCategory()));

    this->connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    this->connect(addButton, SIGNAL(clicked()), this, SLOT(showNewCategoryDialog()));
}

CategoriesDialog::~CategoriesDialog() {}

void CategoriesDialog::onItemClicked(const QModelIndex &index) {
    NewCategoryDialog *dialog = new NewCategoryDialog(this);
    dialog->setWindowTitle(tr("Edit category"));
    dialog->setName(index.data(CategoriesModel::NameRole).toString());
    dialog->setPath(index.data(CategoriesModel::PathRole).toString());
    dialog->open();
    this->connect(dialog, SIGNAL(addCategory(QString,QString)), m_model, SLOT(addCategory(QString,QString)));
}

void CategoriesDialog::showContextMenu(const QPoint &pos) {
    if (m_view->currentIndex().isValid()) {
        m_contextMenu->popup(this->mapToGlobal(pos));
    }
}

void CategoriesDialog::removeCategory() {
    if (m_view->currentIndex().isValid()) {
        m_model->removeCategory(m_view->currentIndex().row());
    }
}

void CategoriesDialog::showNewCategoryDialog() {
    NewCategoryDialog *dialog = new NewCategoryDialog(this);
    dialog->open();
    this->connect(dialog, SIGNAL(addCategory(QString,QString)), m_model, SLOT(addCategory(QString,QString)));
}
