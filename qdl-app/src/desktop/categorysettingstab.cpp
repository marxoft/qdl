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
#include "../shared/categoriesmodel.h"
#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
#include <QTreeView>
#include <QPushButton>
#include <QLineEdit>
#include <QMenu>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

CategorySettingsTab::CategorySettingsTab(QWidget *parent) :
    QWidget(parent),
    m_model(new CategoriesModel(this)),
    m_view(new QTreeView(this)),
    m_contextMenu(new QMenu(this)),
    m_editAction(m_contextMenu->addAction(QIcon::fromTheme("gtk-edit"), tr("Edit"), this, SLOT(editCategory()))),
    m_removeAction(m_contextMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"), this, SLOT(removeCategory()))),
    m_nameEdit(new QLineEdit(this)),
    m_pathEdit(new QLineEdit(this)),
    m_browseButton(new QPushButton(QIcon::fromTheme("document-open"), tr("Browse"), this)),
    m_doneButton(new QPushButton(QIcon::fromTheme("document-save"), tr("Save"), this))
{
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_view, 0, 0, 3, 4);
    grid->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding), 2, 4);
    grid->addWidget(new QLabel(tr("Add/edit category"), this), 3, 0);
    grid->addWidget(new QLabel(QString("%1:").arg(tr("Name")), this), 4, 0);
    grid->addWidget(m_nameEdit, 4, 1);
    grid->addWidget(new QLabel(QString("%1:").arg(tr("Download path")), this), 5, 0);
    grid->addWidget(m_pathEdit, 5, 1);
    grid->addWidget(m_browseButton, 5, 2);
    grid->addWidget(m_doneButton, 6, 0);

    m_editAction->setIconVisibleInMenu(true);
    m_removeAction->setIconVisibleInMenu(true);

    m_doneButton->setEnabled(false);

    m_view->setModel(m_model);
    m_view->setRootIsDecorated(false);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setExpandsOnDoubleClick(false);
    m_view->setItemsExpandable(false);
    m_view->setColumnWidth(0, 200);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);

    this->connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(editCategory()));
    this->connect(m_view, SIGNAL(activated(QModelIndex)), this, SLOT(editCategory()));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    this->connect(m_doneButton, SIGNAL(clicked()), this, SLOT(addCategory()));
    this->connect(m_browseButton, SIGNAL(clicked()), this, SLOT(showFolderDialog()));
    this->connect(m_nameEdit, SIGNAL(textChanged(QString)), this, SLOT(onCategoryEditChanged()));
    this->connect(m_pathEdit, SIGNAL(textChanged(QString)), this, SLOT(onCategoryEditChanged()));
}

void CategorySettingsTab::showContextMenu(const QPoint &pos) {
    m_contextMenu->popup(m_view->mapToGlobal(pos), m_editAction);
}

void CategorySettingsTab::addCategory() {
    m_model->addCategory(m_nameEdit->text(), m_pathEdit->text());

    m_nameEdit->clear();
    m_pathEdit->clear();
}

void CategorySettingsTab::removeCategory() {
    if (m_view->currentIndex().isValid()) {
        m_model->removeCategory(m_view->currentIndex().row());
    }
}

void CategorySettingsTab::editCategory() {
    QModelIndex index = m_view->currentIndex();

    if (index.isValid()) {
        m_nameEdit->setText(index.data(CategoriesModel::NameRole).toString());
        m_pathEdit->setText(index.data(CategoriesModel::PathRole).toString());
    }
}

void CategorySettingsTab::showFolderDialog() {
#if QT_VERSION >= 0x050000
    QString currentPath = m_pathEdit->text().isEmpty() ? QStandardPaths::writableLocation(QDesktopServices::HomeLocation) : m_pathEdit->text();
#else
    QString currentPath = m_pathEdit->text().isEmpty() ? QDesktopServices::storageLocation(QDesktopServices::HomeLocation) : m_pathEdit->text();
#endif
    QString path = QFileDialog::getExistingDirectory(this, tr("Download path"), currentPath);

    if (!path.isEmpty()) {
        m_pathEdit->setText(path);
    }
}

void CategorySettingsTab::onCategoryEditChanged() {
    m_doneButton->setEnabled((!m_nameEdit->text().isEmpty()) && (!m_pathEdit->text().isEmpty()));
}
