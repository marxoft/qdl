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

#include "editdecaptchaaccountdialog.h"
#include "../shared/settings.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>

EditDecaptchaAccountDialog::EditDecaptchaAccountDialog(QWidget *parent) :
    QDialog(parent),
    m_iconLabel(new QLabel(this)),
    m_nameLabel(new QLabel(this)),
    m_userEdit(new QLineEdit(this)),
    m_passEdit(new QLineEdit(this)),
    m_activeCheckbox(new QCheckBox(tr("Use this decaptcha service"), this)),
    m_doneButton(0)
{
    this->setWindowTitle(tr("Decaptcha account"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(QDialogButtonBox::Cancel);
    m_doneButton = buttonBox->addButton(tr("Done"), QDialogButtonBox::ActionRole);
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_iconLabel, 0, 0);
    grid->addWidget(m_nameLabel, 0, 1);
    grid->addWidget(new QLabel(QString("%1/%2").arg(tr("Username")).arg("email"), this), 1, 0, 1, 2);
    grid->addWidget(m_userEdit, 2, 0, 1, 2);
    grid->addWidget(new QLabel(tr("Password"), this), 3, 0, 1, 2);
    grid->addWidget(m_passEdit, 4, 0, 1, 2);
    grid->addWidget(m_activeCheckbox, 5, 0, 1, 2);
    grid->addWidget(buttonBox, 6, 0, 1, 2);

    m_iconLabel->setFixedSize(48, 48);
    m_iconLabel->setScaledContents(true);
    m_passEdit->setEchoMode(QLineEdit::Password);

    this->connect(m_doneButton, SIGNAL(clicked()), this, SLOT(submitAccount()));
    this->connect(m_activeCheckbox, SIGNAL(clicked(bool)), this, SLOT(onActiveChanged(bool)));
    this->connect(m_userEdit, SIGNAL(textEdited(QString)), this, SLOT(onAccountTextChanged()));
    this->connect(m_passEdit, SIGNAL(textEdited(QString)), this, SLOT(onAccountTextChanged()));
    this->connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

EditDecaptchaAccountDialog::~EditDecaptchaAccountDialog() {}

void EditDecaptchaAccountDialog::setAccount(const QString &icon, const QString &serviceName, const QString &username, const QString &password) {
    m_iconLabel->setPixmap(QPixmap(icon).scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_nameLabel->setText(serviceName);
    m_userEdit->setText(username);
    m_passEdit->setText(password);
    m_activeCheckbox->setChecked(Settings::instance()->decaptchaService() == serviceName);
    m_activeCheckbox->setEnabled((!username.isEmpty()) && (!password.isEmpty()));
    m_doneButton->setEnabled(false);
}

void EditDecaptchaAccountDialog::onActiveChanged(bool active) {
    Settings::instance()->setDecaptchaService(active ? m_nameLabel->text() : "");
}

void EditDecaptchaAccountDialog::onAccountTextChanged() {
    bool enable = (!m_userEdit->text().isEmpty()) && (!m_passEdit->text().isEmpty());
    m_doneButton->setEnabled(enable);
    m_activeCheckbox->setEnabled(enable);
}

void EditDecaptchaAccountDialog::submitAccount() {
    emit addAccount(m_nameLabel->text(), m_userEdit->text(), m_passEdit->text());
    this->accept();
}
