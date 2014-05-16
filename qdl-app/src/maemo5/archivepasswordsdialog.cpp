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

#include "archivepasswordsdialog.h"
#include "../shared/archivepasswordsmodel.h"
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QListView>
#include <QMenu>

ArchivePasswordsDialog::ArchivePasswordsDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new ArchivePasswordsModel(this)),
    m_view(new QListView(this)),
    m_contextMenu(new QMenu(this)),
    m_removePasswordAction(m_contextMenu->addAction(tr("Remove"), this, SLOT(removeArchivePassword()))),
    m_passwordEdit(new QLineEdit(this)),
    m_passwordButton(new QPushButton(tr("Add"), this))
{
    this->setWindowTitle(tr("Archive passwords"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    m_view->setModel(m_model);
    m_view->setEditTriggers(QListView::NoEditTriggers);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_passwordEdit->setPlaceholderText(tr("Add password"));

    m_passwordButton->setFixedWidth(150);
    m_passwordButton->setEnabled(false);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_view, 0, 0);
    grid->addWidget(m_passwordEdit, 1, 0);
    grid->addWidget(m_passwordButton, 1, 1);

    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    this->connect(m_passwordEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordEditTextChanged(QString)));
    this->connect(m_passwordEdit, SIGNAL(returnPressed()), this, SLOT(addArchivePassword()));
    this->connect(m_passwordButton, SIGNAL(clicked()), this, SLOT(addArchivePassword()));
}

ArchivePasswordsDialog::~ArchivePasswordsDialog() {}

void ArchivePasswordsDialog::showContextMenu(const QPoint &pos) {
    m_contextMenu->popup(m_view->mapToGlobal(pos), m_removePasswordAction);
}

void ArchivePasswordsDialog::onPasswordEditTextChanged(const QString &text) {
    m_passwordButton->setEnabled(!text.isEmpty());
}

void ArchivePasswordsDialog::addArchivePassword() {
    m_model->addPassword(m_passwordEdit->text());
    m_passwordEdit->clear();
}

void ArchivePasswordsDialog::removeArchivePassword() {
    if (m_view->currentIndex().isValid()) {
        m_model->removePassword(m_view->currentIndex().row());
    }
}
