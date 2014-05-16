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
#include <QMaemo5ValueButton>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QFileDialog>

NewCategoryDialog::NewCategoryDialog(QWidget *parent) :
    QDialog(parent),
    m_nameEdit(new QLineEdit(this)),
    m_pathSelector(new QMaemo5ValueButton(tr("Download path"), this)),
    m_doneButton(new QPushButton(tr("Done"), this))
{
    this->setWindowTitle(tr("New category"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, this);
    buttonBox->addButton(m_doneButton, QDialogButtonBox::AcceptRole);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_nameEdit, 0, 0);
    grid->addWidget(m_pathSelector, 1, 0);
    grid->addWidget(buttonBox, 1, 1);

    m_nameEdit->setPlaceholderText(tr("Name"));
    m_pathSelector->setValueText(tr("None chosen"));
    m_pathSelector->setFocus(Qt::OtherFocusReason);
    m_doneButton->setEnabled(false);

    this->connect(m_nameEdit, SIGNAL(textChanged(QString)), this, SLOT(onNameTextChanged(QString)));
    this->connect(m_pathSelector, SIGNAL(clicked()), this, SLOT(showFileDialog()));
    this->connect(buttonBox, SIGNAL(accepted()), this, SLOT(addCategory()));
}

NewCategoryDialog::~NewCategoryDialog() {}

void NewCategoryDialog::setName(const QString &name) {
    m_nameEdit->setText(name);
}

void NewCategoryDialog::setPath(const QString &path) {
    m_path = path;
    m_pathSelector->setValueText(path);
}

void NewCategoryDialog::onNameTextChanged(const QString &text) {
    m_doneButton->setEnabled((!text.isEmpty()) && (!m_path.isEmpty()));
}

void NewCategoryDialog::showFileDialog() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose folder"), m_path.isEmpty() ? Settings::instance()->downloadPath() : m_path);

    if (!path.isEmpty()) {
        m_path = path;
        m_pathSelector->setValueText(path);
        m_doneButton->setEnabled(!m_nameEdit->text().isEmpty());
    }
    else {
        m_doneButton->setEnabled((!m_path.isEmpty()) && (!m_nameEdit->text().isEmpty()));
    }
}

void NewCategoryDialog::addCategory() {
    emit addCategory(m_nameEdit->text(), m_path);
    this->accept();
}
