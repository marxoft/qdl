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

#include "retrieveurlsdialog.h"
#include "../shared/settings.h"
#include "../shared/database.h"
#include <QTextEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QDropEvent>
#include <QUrl>

RetrieveUrlsDialog::RetrieveUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_urlsEdit(new QTextEdit(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_vbox(new QVBoxLayout(this))
{
    this->setWindowTitle(tr("Retrieve URLs"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setMinimumSize(500, 300);
    this->setAcceptDrops(true);

    m_buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme("dialog-ok"));
    m_buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QIcon::fromTheme("dialog-cancel"));

    m_vbox->addWidget(m_urlsEdit);
    m_vbox->addWidget(m_buttonBox);

    this->connect(m_urlsEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    this->connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    this->connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

RetrieveUrlsDialog::~RetrieveUrlsDialog() {}

void RetrieveUrlsDialog::dragEnterEvent(QDragEnterEvent *event) {
    if ((event->mimeData()->hasUrls()) && (event->mimeData()->urls().first().path().toLower().endsWith(".txt"))) {
        event->acceptProposedAction();
    }
}

void RetrieveUrlsDialog::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        QString fileName = event->mimeData()->urls().first().path();

        if ((QFile::exists(fileName)) && (fileName.toLower().endsWith(".txt"))) {
            this->parseUrlsFromTextFile(fileName);
        }
    }
}

QString RetrieveUrlsDialog::text() const {
    return m_urlsEdit->toPlainText();
}

void RetrieveUrlsDialog::setText(const QString &text) {
    m_urlsEdit->setText(text);
}

void RetrieveUrlsDialog::parseUrlsFromTextFile(const QString &fileName) {
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

void RetrieveUrlsDialog::accept() {
    emit urlsAvailable(this->text());
    QDialog::accept();
}

void RetrieveUrlsDialog::onTextChanged() {
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!this->text().isEmpty());
}
