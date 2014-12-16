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
#include "../shared/decaptchaaccountsmodel.h"
#include "../shared/settings.h"
#include <QTreeView>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>

CaptchaSettingsTab::CaptchaSettingsTab(QWidget *parent) :
    QWidget(parent),
    m_model(new DecaptchaAccountsModel(this)),
    m_view(new QTreeView(this)),
    m_contextMenu(new QMenu(this)),
    m_editAction(m_contextMenu->addAction(QIcon::fromTheme("gtk-edit"), tr("Edit"), this, SLOT(editAccount()))),
    m_removeAction(m_contextMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"), this, SLOT(removeAccount()))),
    m_nameEdit(new QLineEdit(this)),
    m_passEdit(new QLineEdit(this)),
    m_doneButton(new QPushButton(QIcon::fromTheme("document-save"), tr("Save"), this)),
    m_checkbox(new QCheckBox(tr("Use decaptcha service"), this))
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
    grid->addWidget(m_checkbox, 6, 1);

    m_editAction->setIconVisibleInMenu(true);
    m_removeAction->setIconVisibleInMenu(true);
    
    m_passEdit->setEchoMode(QLineEdit::Password);

    m_doneButton->setEnabled(false);

    m_view->setModel(m_model);
    m_view->setRootIsDecorated(false);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);
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
    this->connect(m_checkbox, SIGNAL(clicked(bool)), this, SLOT(onCheckboxClicked(bool)));
}

CaptchaSettingsTab::~CaptchaSettingsTab() {}

void CaptchaSettingsTab::showContextMenu(const QPoint &pos) {
    m_contextMenu->popup(m_view->mapToGlobal(pos), m_editAction);
}

void CaptchaSettingsTab::onCheckboxClicked(bool checked) {
    if (checked) {
        if (m_view->currentIndex().isValid()) {
            Settings::instance()->setDecaptchaService(m_view->currentIndex().data(DecaptchaAccountsModel::ServiceNameRole).toString());
        }
    }
    else {
        Settings::instance()->setDecaptchaService(QString());
    }
}

void CaptchaSettingsTab::addAccount() {
    if (m_view->currentIndex().isValid()) {
        m_model->addAccount(m_view->currentIndex().data(DecaptchaAccountsModel::ServiceNameRole).toString(),
                            m_nameEdit->text(), m_passEdit->text());
    }

    m_nameEdit->clear();
    m_passEdit->clear();
}

void CaptchaSettingsTab::removeAccount() {
    if (m_view->currentIndex().isValid()) {
        m_model->removeAccount(m_view->currentIndex().row());
    }
}

void CaptchaSettingsTab::editAccount() {
    QModelIndex index = m_view->currentIndex();

    if (index.isValid()) {
        m_nameEdit->setText(index.data(DecaptchaAccountsModel::UsernameRole).toString());
        m_passEdit->setText(index.data(DecaptchaAccountsModel::PasswordRole).toString());
        m_checkbox->setChecked(Settings::instance()->decaptchaService()
                               == index.data(DecaptchaAccountsModel::ServiceNameRole).toString());
    }
}

void CaptchaSettingsTab::onAccountEditChanged() {
    m_doneButton->setEnabled((!m_nameEdit->text().isEmpty()) && (!m_passEdit->text().isEmpty()));
}
