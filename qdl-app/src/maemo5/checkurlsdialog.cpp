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

#include "checkurlsdialog.h"
#include "../shared/urlchecker.h"
#include <QGridLayout>
#include <QShowEvent>
#include <QDialogButtonBox>
#include <QTreeView>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>

CheckUrlsDialog::CheckUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_view(new QTreeView(this)),
    m_progressBar(new QProgressBar(this)),
    m_infoLabel(new QLabel(QString("<i>%1</i>").arg(tr("Checking URLs")), this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Vertical, this))
{
    this->setWindowTitle(tr("Check URLs"));
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setFixedHeight(340);

    m_view->setModel(UrlChecker::instance()->model());
    m_view->setColumnWidth(0, 520);
    m_view->setColumnWidth(1, 50);
    m_view->setRootIsDecorated(false);
    m_view->setSelectionMode(QTreeView::NoSelection);
    m_view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    m_progressBar->setValue(0);
    m_progressBar->setMaximum(100);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_view, 0, 0);
    grid->addWidget(m_progressBar, 1, 0, Qt::AlignBottom);
    grid->addWidget(m_infoLabel, 2, 0, Qt::AlignBottom);
    grid->addWidget(m_buttonBox, 0, 1, 3, 1, Qt::AlignBottom);

    this->connect(UrlChecker::instance(), SIGNAL(progressChanged(int)), this, SLOT(onProgressChanged(int)));
    this->connect(UrlChecker::instance(), SIGNAL(canceled()), this, SLOT(accept()));
    this->connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    this->connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CheckUrlsDialog::~CheckUrlsDialog() {}

void CheckUrlsDialog::accept() {
	QDialog::accept();
	UrlChecker::instance()->model()->clear();
}

void CheckUrlsDialog::reject() {
	QDialog::reject();
	
	if (UrlChecker::instance()->progress() < 100) {
		UrlChecker::instance()->cancel();
	}
	else {
		UrlChecker::instance()->model()->clear();
	}
}

void CheckUrlsDialog::showEvent(QShowEvent *event) {
   QDialog::showEvent(event);
   this->resetDialog();
   this->onProgressChanged(UrlChecker::instance()->progress());
}

void CheckUrlsDialog::onProgressChanged(int progress) {
    m_progressBar->setValue(progress);

    if (progress == 100) {
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        m_infoLabel->setText(QString("<i>%1</i>").arg(tr("Completed")));
    }
}

void CheckUrlsDialog::resetDialog() {
    m_progressBar->setValue(0);
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    m_infoLabel->setText(QString("<i>%1</i>").arg(tr("Checking URLs")));
}
