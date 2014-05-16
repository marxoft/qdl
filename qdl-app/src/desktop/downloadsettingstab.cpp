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
#include "../shared/archivepasswordsmodel.h"
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QFileDialog>
#include <QListView>
#include <QMenu>

DownloadSettingsTab::DownloadSettingsTab(QWidget *parent) :
    QWidget(parent),
    m_pathEdit(new QLineEdit(this)),
    m_statusCheckbox(new QCheckBox(tr("Start downloads automatically"), this)),
    m_clipboardCheckbox(new QCheckBox(tr("Monitor clipboard for URLs"), this)),
    m_extractCheckbox(new QCheckBox(tr("Extract downloaded archives"), this)),
    m_subfoldersCheckbox(new QCheckBox(tr("Create subfolders when extracting archives"))),
    m_deleteCheckbox(new QCheckBox(tr("Delete extracted archives"), this)),
    m_model(new ArchivePasswordsModel(this)),
    m_view(new QListView(this)),
    m_contextMenu(new QMenu(this)),
    m_removePasswordAction(m_contextMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"), this, SLOT(removeArchivePassword()))),
    m_passwordEdit(new QLineEdit(this)),
    m_passwordButton(new QPushButton(QIcon::fromTheme("list-add"), tr("Add"), this))
{
    QPushButton *browseButton = new QPushButton(QIcon::fromTheme("document-open"), tr("Browse"), this);

    bool transferQueueIsEmpty = TransferModel::instance()->rowCount() == 0;

    m_pathEdit->setEnabled(transferQueueIsEmpty);
    browseButton->setEnabled(transferQueueIsEmpty);

    m_view->setModel(m_model);
    m_view->setEditTriggers(QListView::NoEditTriggers);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);

    m_removePasswordAction->setIconVisibleInMenu(true);

    m_passwordEdit->setPlaceholderText(tr("Add password"));
    m_passwordButton->setEnabled(false);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(new QLabel(tr("Default download path") + ':', this), 0, 0);
    grid->addWidget(m_pathEdit, 0, 1);
    grid->addWidget(browseButton, 0, 2);
    grid->addWidget(m_statusCheckbox, 1, 0);
    grid->addWidget(m_clipboardCheckbox, 2, 0);
    grid->addWidget(m_extractCheckbox, 3, 0);
    grid->addWidget(m_subfoldersCheckbox, 4, 0);
    grid->addWidget(m_deleteCheckbox, 5, 0);
    grid->addWidget(new QLabel(tr("Archive passwords") + ":", this), 6, 0, 1, 3);
    grid->addWidget(m_view, 7, 0, 2, 1);
    grid->addWidget(m_passwordEdit, 8, 1);
    grid->addWidget(m_passwordButton, 8, 2);

    this->connect(browseButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));
    this->connect(m_extractCheckbox, SIGNAL(toggled(bool)), m_deleteCheckbox, SLOT(setEnabled(bool)));
    this->connect(m_extractCheckbox, SIGNAL(toggled(bool)), m_subfoldersCheckbox, SLOT(setEnabled(bool)));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    this->connect(m_passwordEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordEditTextChanged(QString)));
    this->connect(m_passwordEdit, SIGNAL(returnPressed()), this, SLOT(addArchivePassword()));
    this->connect(m_passwordButton, SIGNAL(clicked()), this, SLOT(addArchivePassword()));

    this->loadSettings();
}

DownloadSettingsTab::~DownloadSettingsTab() {}

void DownloadSettingsTab::loadSettings() {
    m_pathEdit->setText(Settings::instance()->downloadPath());
    m_statusCheckbox->setChecked(Settings::instance()->startTransfersAutomatically());
    m_clipboardCheckbox->setChecked(Settings::instance()->monitorClipboard());
    m_extractCheckbox->setChecked(Settings::instance()->extractDownloadedArchives());
    m_subfoldersCheckbox->setChecked(Settings::instance()->createSubfolderForArchives());
    m_subfoldersCheckbox->setEnabled(m_extractCheckbox->isChecked());
    m_deleteCheckbox->setChecked(Settings::instance()->deleteExtractedArchives());
    m_deleteCheckbox->setEnabled(m_extractCheckbox->isChecked());
}

void DownloadSettingsTab::saveSettings() {
    Settings::instance()->setDownloadPath(m_pathEdit->text());
    Settings::instance()->setStartTransfersAutomatically(m_statusCheckbox->isChecked());
    Settings::instance()->setMonitorClipboard(m_clipboardCheckbox->isChecked());
    Settings::instance()->setExtractDownloadedArchives(m_extractCheckbox->isChecked());
    Settings::instance()->setCreateSubfolderForArchives(m_subfoldersCheckbox->isChecked());
    Settings::instance()->setDeleteExtractedArchives(m_deleteCheckbox->isChecked());
}

void DownloadSettingsTab::showFileDialog() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose folder"), Settings::instance()->downloadPath());

    if (!path.isEmpty()) {
        m_pathEdit->setText(path);
    }
}

void DownloadSettingsTab::showContextMenu(const QPoint &pos) {
    m_contextMenu->popup(m_view->mapToGlobal(pos), m_removePasswordAction);
}

void DownloadSettingsTab::onPasswordEditTextChanged(const QString &text) {
    m_passwordButton->setEnabled(!text.isEmpty());
}

void DownloadSettingsTab::addArchivePassword() {
    m_model->addPassword(m_passwordEdit->text());
    m_passwordEdit->clear();
}

void DownloadSettingsTab::removeArchivePassword() {
    if (m_view->currentIndex().isValid()) {
        m_model->removePassword(m_view->currentIndex().row());
    }
}
