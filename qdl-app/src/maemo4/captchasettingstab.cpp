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

#include "captchasettingstab.h"
#include "editdecaptchaaccountdialog.h"
#include "../shared/decaptchaaccountsmodel.h"
#include <QTreeView>
#include <QVBoxLayout>
#include <QMenu>

CaptchaSettingsTab::CaptchaSettingsTab(QWidget *parent) :
    QWidget(parent),
    m_model(new DecaptchaAccountsModel(this)),
    m_view(new QTreeView(this)),
    m_contextMenu(new QMenu(this)),
    m_editAction(m_contextMenu->addAction(tr("Edit"), this, SLOT(editAccount()))),
    m_removeAction(m_contextMenu->addAction(tr("Remove"), this, SLOT(removeAccount())))
{
    m_view->setModel(m_model);
    m_view->setRootIsDecorated(false);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setExpandsOnDoubleClick(false);
    m_view->setItemsExpandable(false);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setColumnWidth(0, 200);
    m_view->setColumnWidth(1, 200);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(m_view);

    this->connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(editAccount()));
    this->connect(m_view, SIGNAL(activated(QModelIndex)), this, SLOT(editAccount()));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

CaptchaSettingsTab::~CaptchaSettingsTab() {}

void CaptchaSettingsTab::showContextMenu(const QPoint &pos) {
    m_contextMenu->popup(m_view->mapToGlobal(pos), m_editAction);
}

void CaptchaSettingsTab::removeAccount() {
    if (m_view->currentIndex().isValid()) {
        m_model->removeAccount(m_view->currentIndex().row());
    }
}

void CaptchaSettingsTab::editAccount() {
    QModelIndex index = m_view->currentIndex();

    if (index.isValid()) {
        EditDecaptchaAccountDialog *dialog = new EditDecaptchaAccountDialog(this);
        dialog->setAccount(index.data(DecaptchaAccountsModel::ServiceIconRole).toString(),
                           index.data(DecaptchaAccountsModel::ServiceNameRole).toString(),
                           index.data(DecaptchaAccountsModel::UsernameRole).toString(),
                           index.data(DecaptchaAccountsModel::PasswordRole).toString());

        dialog->open();
        this->connect(dialog, SIGNAL(addAccount(QString,QString,QString)), m_model, SLOT(addAccount(QString,QString,QString)));
    }
}
