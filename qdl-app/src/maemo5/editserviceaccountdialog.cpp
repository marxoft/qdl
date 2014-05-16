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

#include "editserviceaccountdialog.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>

EditServiceAccountDialog::EditServiceAccountDialog(QWidget *parent) :
    QDialog(parent),
    m_iconLabel(new QLabel(this)),
    m_nameLabel(new QLabel(this)),
    m_userEdit(new QLineEdit(this)),
    m_passEdit(new QLineEdit(this)),
    m_doneButton(0)
{
    this->setWindowTitle(tr("Service account"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, this);
    m_doneButton = buttonBox->addButton(tr("Done"), QDialogButtonBox::ActionRole);
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_iconLabel, 0, 0);
    grid->addWidget(m_nameLabel, 0, 1);
    grid->addWidget(new QLabel(QString("%1/%2").arg(tr("Username")).arg("email"), this), 1, 0, 1, 2);
    grid->addWidget(m_userEdit, 2, 0, 1, 2);
    grid->addWidget(new QLabel(tr("Password"), this), 3, 0, 1, 2);
    grid->addWidget(m_passEdit, 4, 0, 1, 2);
    grid->addWidget(buttonBox, 4, 2);

    m_iconLabel->setFixedSize(48, 48);
    m_iconLabel->setScaledContents(true);
    m_passEdit->setEchoMode(QLineEdit::Password);

    this->connect(m_doneButton, SIGNAL(clicked()), this, SLOT(submitAccount()));
    this->connect(m_userEdit, SIGNAL(textEdited(QString)), this, SLOT(onAccountTextChanged()));
    this->connect(m_passEdit, SIGNAL(textEdited(QString)), this, SLOT(onAccountTextChanged()));
}

EditServiceAccountDialog::~EditServiceAccountDialog() {}

void EditServiceAccountDialog::setAccount(const QString &icon, const QString &serviceName, const QString &username, const QString &password) {
    m_iconLabel->setPixmap(QPixmap(icon).scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_nameLabel->setText(serviceName);
    m_userEdit->setText(username);
    m_passEdit->setText(password);
    m_doneButton->setEnabled(false);
}

void EditServiceAccountDialog::onAccountTextChanged() {
    m_doneButton->setEnabled((!m_userEdit->text().isEmpty()) && (!m_passEdit->text().isEmpty()));
}

void EditServiceAccountDialog::submitAccount() {
    emit addAccount(m_nameLabel->text(), m_userEdit->text(), m_passEdit->text());
    this->accept();
}
