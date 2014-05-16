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

#include "serviceaccountsdialog.h"
#include "editserviceaccountdialog.h"
#include "../shared/serviceaccountsmodel.h"
#include <QTreeView>
#include <QMenu>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGridLayout>

ServiceAccountsDialog::ServiceAccountsDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new ServiceAccountsModel(this)),
    m_view(new QTreeView(this)),
    m_contextMenu(new QMenu(this))
{
    this->setWindowTitle(tr("Service accounts"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setFixedHeight(340);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_view, 0, 0);

    m_contextMenu->addAction(tr("Remove"), this, SLOT(removeAccount()));

    m_view->setModel(m_model);
    m_view->setRootIsDecorated(false);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setExpandsOnDoubleClick(false);
    m_view->setItemsExpandable(false);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setColumnWidth(0, 300);
    m_view->setColumnWidth(1, 300);

    this->connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

void ServiceAccountsDialog::onItemClicked(const QModelIndex &index) {
    EditServiceAccountDialog *dialog = new EditServiceAccountDialog(this);
    dialog->setAccount(index.data(ServiceAccountsModel::ServiceIconRole).toString(),
                       index.data(ServiceAccountsModel::ServiceNameRole).toString(),
                       index.data(ServiceAccountsModel::UsernameRole).toString(),
                       index.data(ServiceAccountsModel::PasswordRole).toString());

    dialog->open();
    this->connect(dialog, SIGNAL(addAccount(QString,QString,QString)), m_model, SLOT(addAccount(QString,QString,QString)));
}

void ServiceAccountsDialog::showContextMenu(const QPoint &pos) {
    if (m_view->currentIndex().isValid()) {
        m_contextMenu->popup(mapToGlobal(pos));
    }
}

void ServiceAccountsDialog::removeAccount() {
    if (m_view->currentIndex().isValid()) {
        m_model->removeAccount(m_view->currentIndex().row());
    }
}
