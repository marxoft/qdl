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

#include "downloadsettingstab.h"
#include "../shared/settings.h"
#include "../shared/transfermodel.h"
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QFileDialog>

DownloadSettingsTab::DownloadSettingsTab(QWidget *parent) :
    QWidget(parent),
    m_pathEdit(new QLineEdit(this)),
    m_statusCheckbox(new QCheckBox(tr("Start downloads automatically"), this)),
    m_clipboardCheckbox(new QCheckBox(tr("Monitor clipboard for URLs"), this))
{
    QPushButton *browseButton = new QPushButton(tr("Browse"), this);

    bool transferQueueIsEmpty = TransferModel::instance()->rowCount() == 0;

    m_pathEdit->setEnabled(transferQueueIsEmpty);
    browseButton->setEnabled(transferQueueIsEmpty);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(new QLabel(tr("Download path") + ':', this), 0, 0);
    grid->addWidget(m_pathEdit, 0, 1);
    grid->addWidget(browseButton, 0, 2);
    grid->addWidget(m_statusCheckbox, 1, 0, 1, 3);
    grid->addWidget(m_clipboardCheckbox, 2, 0, 1, 3);

    this->connect(browseButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));

    this->loadSettings();
}

DownloadSettingsTab::~DownloadSettingsTab() {}

void DownloadSettingsTab::loadSettings() {
    m_pathEdit->setText(Settings::instance()->downloadPath());
    m_statusCheckbox->setChecked(Settings::instance()->startTransfersAutomatically());
    m_clipboardCheckbox->setChecked(Settings::instance()->monitorClipboard());
}

void DownloadSettingsTab::saveSettings() {
    Settings::instance()->setDownloadPath(m_pathEdit->text());
    Settings::instance()->setStartTransfersAutomatically(m_statusCheckbox->isChecked());
    Settings::instance()->setMonitorClipboard(m_clipboardCheckbox->isChecked());
}

void DownloadSettingsTab::showFileDialog() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose folder"), Settings::instance()->downloadPath());

    if (!path.isEmpty()) {
        m_pathEdit->setText(path);
    }
}
