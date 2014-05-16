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

#include "newcategorydialog.h"
#include "../shared/settings.h"
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>

NewCategoryDialog::NewCategoryDialog(QWidget *parent) :
    QDialog(parent),
    m_nameEdit(new QLineEdit(this)),
    m_pathEdit(new QLineEdit(this)),
    m_browseButton(new QPushButton(tr("Browse"), this)),
    m_doneButton(new QPushButton(tr("Done"), this))
{
    this->setWindowTitle(tr("New category"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(m_browseButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);
    buttonBox->addButton(m_doneButton, QDialogButtonBox::AcceptRole);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(new QLabel(tr("Name") + ":", this), 0, 0);
    grid->addWidget(m_nameEdit, 0, 1);
    grid->addWidget(new QLabel(tr("Download path") + ":", this), 1, 0);
    grid->addWidget(m_pathEdit, 1, 1);
    grid->addWidget(buttonBox, 2, 0, 1, 2);

    m_doneButton->setEnabled(false);

    this->connect(m_nameEdit, SIGNAL(textChanged(QString)), this, SLOT(onNameTextChanged(QString)));
    this->connect(m_browseButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));
    this->connect(buttonBox, SIGNAL(accepted()), this, SLOT(addCategory()));
    this->connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

NewCategoryDialog::~NewCategoryDialog() {}

void NewCategoryDialog::setName(const QString &name) {
    m_nameEdit->setText(name);
}

void NewCategoryDialog::setPath(const QString &path) {
    m_pathEdit->setText(path);
}

void NewCategoryDialog::onNameTextChanged(const QString &text) {
    m_doneButton->setEnabled((!text.isEmpty()) && (!m_pathEdit->text().isEmpty()));
}

void NewCategoryDialog::showFileDialog() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose folder"), m_pathEdit->text().isEmpty() ? Settings::instance()->downloadPath() : m_pathEdit->text());

    if (!path.isEmpty()) {
        m_pathEdit->setText(path);
        m_doneButton->setEnabled(!m_nameEdit->text().isEmpty());
    }
    else {
        m_doneButton->setEnabled((!m_pathEdit->text().isEmpty()) && (!m_nameEdit->text().isEmpty()));
    }
}

void NewCategoryDialog::addCategory() {
    emit addCategory(m_nameEdit->text(), m_pathEdit->text());
    this->accept();
}
