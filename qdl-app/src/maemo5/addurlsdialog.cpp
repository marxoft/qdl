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

#include "addurlsdialog.h"
#include "valueselector.h"
#include "../shared/settings.h"
#include "../shared/database.h"
#include "../shared/selectionmodels.h"
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFile>
#include <QUrl>

AddUrlsDialog::AddUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_urlsEdit(new QTextEdit(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Vertical, this))
{
    this->setWindowTitle(tr("Add URLs"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    ValueSelector *categorySelector = new ValueSelector(tr("Category"), this);
    SelectionModel *model = new SelectionModel(this);

    foreach (QString category, Database::instance()->getCategoryNames()) {
        model->addItem(category, category);
    }

    categorySelector->setModel(model);
    categorySelector->setValue(Settings::instance()->defaultCategory());
    categorySelector->setEnabled(model->rowCount() > 0);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_urlsEdit, 0, 0);
    grid->addWidget(categorySelector, 1, 0);
    grid->addWidget(m_buttonBox, 1, 1);

    this->connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    this->connect(m_urlsEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    this->connect(categorySelector, SIGNAL(valueChanged(QVariant)), this, SLOT(setCategory(QVariant)));
}

AddUrlsDialog::~AddUrlsDialog() {}

QString AddUrlsDialog::text() const {
    return m_urlsEdit->toPlainText();
}

void AddUrlsDialog::setText(const QString &text) {
    m_urlsEdit->setText(text);
}

void AddUrlsDialog::parseUrlsFromTextFile(const QString &fileName) {
    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!file.atEnd()) {
            QUrl url(file.readLine());

            if (url.isValid()) {
                m_urlsEdit->insertPlainText(url.toString());
            }
        }

        file.close();
    }
}

void AddUrlsDialog::accept() {
    emit urlsAvailable(this->text());
    QDialog::accept();
}

void AddUrlsDialog::onTextChanged() {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!this->text().isEmpty());
}

void AddUrlsDialog::setCategory(const QVariant &value) {
    Settings::instance()->setDefaultCategory(value.toString());
}
