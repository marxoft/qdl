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
#include "../shared/settings.h"
#include "../shared/database.h"
#include "../shared/pluginmanager.h"
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGridLayout>
#include <QFile>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

AddUrlsDialog::AddUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_urlsEdit(new QTextEdit(this)),
    m_serviceComboBox(new QComboBox(this)),
    m_categoryComboBox(new QComboBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_grid(new QGridLayout(this))
{
    this->setWindowTitle(tr("Add URLs"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setMinimumSize(500, 300);
    this->setAcceptDrops(true);

    m_serviceComboBox->addItems(QStringList(tr("Detect from URL")) << PluginManager::instance()->servicePluginNames());

    m_categoryComboBox->addItems(Database::instance()->getCategoryNames());
    m_categoryComboBox->setCurrentIndex(m_categoryComboBox->findText(Settings::instance()->defaultCategory()));
    m_categoryComboBox->setEnabled(m_categoryComboBox->count() > 0);

    m_buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme("dialog-ok"));
    m_buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QIcon::fromTheme("dialog-cancel"));

    m_grid->addWidget(m_urlsEdit, 0, 0, 1, 3);
    m_grid->addWidget(new QLabel(tr("Service") + ":", this), 1, 0);
    m_grid->addWidget(m_serviceComboBox, 1, 1);
    m_grid->addWidget(new QLabel(tr("Category") + ":", this), 2, 0);
    m_grid->addWidget(m_categoryComboBox, 2, 1);
    m_grid->addWidget(m_buttonBox, 2, 2);
    m_grid->setColumnStretch(2, 2);

    this->connect(m_urlsEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    this->connect(m_categoryComboBox, SIGNAL(currentIndexChanged(QString)), Settings::instance(), SLOT(setDefaultCategory(QString)));
    this->connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    this->connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

AddUrlsDialog::~AddUrlsDialog() {}

void AddUrlsDialog::dragEnterEvent(QDragEnterEvent *event) {
    if ((event->mimeData()->hasUrls()) && (event->mimeData()->urls().first().path().toLower().endsWith(".txt"))) {
        event->acceptProposedAction();
    }
}

void AddUrlsDialog::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        QString fileName = event->mimeData()->urls().first().path();

        if ((QFile::exists(fileName)) && (fileName.toLower().endsWith(".txt"))) {
            this->parseUrlsFromTextFile(fileName);
        }
    }
}

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
    emit urlsAvailable(this->text(), m_serviceComboBox->currentIndex() == 0 ? QString() 
                                                                            : m_serviceComboBox->currentText());
    QDialog::accept();
}

void AddUrlsDialog::onTextChanged() {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!this->text().isEmpty());
}
