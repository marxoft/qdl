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

#include "accountsettingstab.h"
#include "../shared/serviceaccountsmodel.h"
#include <QGridLayout>
#include <QLabel>
#include <QTreeView>
#include <QPushButton>
#include <QLineEdit>
#include <QMenu>

AccountSettingsTab::AccountSettingsTab(QWidget *parent) :
    QWidget(parent),
    m_model(new ServiceAccountsModel(this)),
    m_view(new QTreeView(this)),
    m_contextMenu(new QMenu(this)),
    m_editAction(m_contextMenu->addAction(QIcon::fromTheme("gtk-edit"), tr("Edit"), this, SLOT(editAccount()))),
    m_removeAction(m_contextMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"), this, SLOT(removeAccount()))),
    m_nameEdit(new QLineEdit(this)),
    m_passEdit(new QLineEdit(this)),
    m_doneButton(new QPushButton(QIcon::fromTheme("document-save"), tr("Save"), this))
{
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_view, 0, 0, 3, 4);
    grid->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding), 2, 4);
    grid->addWidget(new QLabel(tr("Add/edit account"), this), 3, 0);
    grid->addWidget(new QLabel(QString("%1:").arg(tr("Username")), this), 4, 0);
    grid->addWidget(m_nameEdit, 4, 1);
    grid->addWidget(new QLabel(QString("%1:").arg(tr("Password")), this), 5, 0);
    grid->addWidget(m_passEdit, 5, 1);
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
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setColumnWidth(0, 200);
    m_view->setColumnWidth(1, 200);

    this->connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(editAccount()));
    this->connect(m_view, SIGNAL(activated(QModelIndex)), this, SLOT(editAccount()));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    this->connect(m_doneButton, SIGNAL(clicked()), this, SLOT(addAccount()));
    this->connect(m_nameEdit, SIGNAL(textChanged(QString)), this, SLOT(onAccountEditChanged()));
    this->connect(m_passEdit, SIGNAL(textChanged(QString)), this, SLOT(onAccountEditChanged()));
}

AccountSettingsTab::~AccountSettingsTab() {}

void AccountSettingsTab::showContextMenu(const QPoint &pos) {
    m_contextMenu->popup(m_view->mapToGlobal(pos), m_editAction);
}

void AccountSettingsTab::addAccount() {
    if (m_view->currentIndex().isValid()) {
        m_model->addAccount(m_view->currentIndex().data(ServiceAccountsModel::ServiceNameRole).toString(), m_nameEdit->text(), m_passEdit->text());
    }

    m_nameEdit->clear();
    m_passEdit->clear();
}

void AccountSettingsTab::removeAccount() {
    if (m_view->currentIndex().isValid()) {
        m_model->removeAccount(m_view->currentIndex().row());
    }
}

void AccountSettingsTab::editAccount() {
    QModelIndex index = m_view->currentIndex();

    if (index.isValid()) {
        m_nameEdit->setText(index.data(ServiceAccountsModel::UsernameRole).toString());
        m_passEdit->setText(index.data(ServiceAccountsModel::PasswordRole).toString());
    }
}

void AccountSettingsTab::onAccountEditChanged() {
    m_doneButton->setEnabled((!m_nameEdit->text().isEmpty()) && (!m_passEdit->text().isEmpty()));
}
