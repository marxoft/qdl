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
#include <QHeaderView>
#include <QTreeView>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QHideEvent>

CheckUrlsDialog::CheckUrlsDialog(QWidget *parent) :
    QDialog(parent),
    m_view(new QTreeView(this)),
    m_progressBar(new QProgressBar(this)),
    m_infoLabel(new QLabel(QString("<i>%1</i>").arg(tr("Checking URLs")), this)),
    m_buttonBox(new QDialogButtonBox(Qt::Horizontal, this)),
    m_okButton(m_buttonBox->addButton(QDialogButtonBox::Ok)),
    m_cancelButton(m_buttonBox->addButton(QDialogButtonBox::Cancel))
{
    this->setWindowTitle(tr("Check URLs"));
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setMinimumSize(600, 400);

    m_view->setModel(UrlChecker::instance()->model());
    m_view->setRootIsDecorated(false);
    m_view->setSelectionMode(QTreeView::NoSelection);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);
    m_view->header()->setStretchLastSection(false);
    m_view->header()->resizeSection(1, 40);
#if QT_VERSION >= 0x050000
    m_view->header()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
    m_view->header()->setResizeMode(0, QHeaderView::Stretch);
#endif

    m_progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_progressBar->setValue(0);
    m_progressBar->setMaximum(100);

    m_okButton->setEnabled(false);
    m_okButton->setIcon(QIcon::fromTheme("dialog-ok"));
    m_cancelButton->setIcon(QIcon::fromTheme("process-stop"));

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_view, 0, 0);
    grid->addWidget(m_progressBar, 1, 0);
    grid->addWidget(m_infoLabel, 2, 0);
    grid->addWidget(m_buttonBox, 3, 0);

    this->connect(UrlChecker::instance(), SIGNAL(progressChanged(int)), this, SLOT(onProgressChanged(int)));
    this->connect(UrlChecker::instance(), SIGNAL(canceled()), this, SLOT(onCanceled()));
    this->connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    this->connect(m_buttonBox, SIGNAL(rejected()), UrlChecker::instance(), SLOT(cancel()));
}

CheckUrlsDialog::~CheckUrlsDialog() {}

void CheckUrlsDialog::hideEvent(QHideEvent *event) {
    this->resetDialog();
    QDialog::hideEvent(event);
}

void CheckUrlsDialog::onProgressChanged(int progress) {
    m_progressBar->setValue(progress);

    if (progress == 100) {
        m_okButton->setEnabled(true);
        m_cancelButton->setEnabled(false);
        m_infoLabel->setText(QString("<i>%1</i>").arg(tr("Completed")));
    }
}

void CheckUrlsDialog::onCanceled() {
    m_okButton->setEnabled(true);
    m_cancelButton->setEnabled(false);
    m_infoLabel->setText(QString("<i>%1</i>").arg(tr("Canceled")));
}

void CheckUrlsDialog::resetDialog() {
    m_progressBar->setValue(0);
    m_okButton->setEnabled(false);
    m_cancelButton->setEnabled(true);
    m_infoLabel->setText(QString("<i>%1</i>").arg(tr("Checking URLs")));
    UrlChecker::instance()->model()->clear();
}
