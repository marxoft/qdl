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

#include "archivesettingstab.h"
#include "../shared/settings.h"
#include "../shared/archivepasswordsmodel.h"
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QFileDialog>
#include <QListView>
#include <QMenu>

ArchiveSettingsTab::ArchiveSettingsTab(QWidget *parent) :
    QWidget(parent),
    m_extractCheckbox(new QCheckBox(tr("Extract downloaded archives"), this)),
    m_subfoldersCheckbox(new QCheckBox(tr("Create subfolders when extracting archives"))),
    m_deleteCheckbox(new QCheckBox(tr("Delete extracted archives"), this)),
    m_model(new ArchivePasswordsModel(this)),
    m_view(new QListView(this)),
    m_contextMenu(new QMenu(this)),
    m_removePasswordAction(m_contextMenu->addAction(tr("Remove"), this, SLOT(removeArchivePassword()))),
    m_passwordEdit(new QLineEdit(this)),
    m_passwordButton(new QPushButton(tr("Add"), this))
{
    m_view->setModel(m_model);
    m_view->setEditTriggers(QListView::NoEditTriggers);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);

    m_passwordButton->setEnabled(false);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_extractCheckbox, 1, 0, 1, 3);
    grid->addWidget(m_subfoldersCheckbox, 2, 0, 1, 3);
    grid->addWidget(m_deleteCheckbox, 3, 0, 1, 3);
    grid->addWidget(new QLabel(tr("Archive passwords") + ":", this), 4, 0, 1, 3);
    grid->addWidget(m_view, 5, 0, 2, 1);
    grid->addWidget(m_passwordEdit, 6, 1);
    grid->addWidget(m_passwordButton, 6, 2);

    this->connect(m_extractCheckbox, SIGNAL(toggled(bool)), m_deleteCheckbox, SLOT(setEnabled(bool)));
    this->connect(m_extractCheckbox, SIGNAL(toggled(bool)), m_subfoldersCheckbox, SLOT(setEnabled(bool)));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    this->connect(m_passwordEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordEditTextChanged(QString)));
    this->connect(m_passwordEdit, SIGNAL(returnPressed()), this, SLOT(addArchivePassword()));
    this->connect(m_passwordButton, SIGNAL(clicked()), this, SLOT(addArchivePassword()));

    this->loadSettings();
}

ArchiveSettingsTab::~ArchiveSettingsTab() {}

void ArchiveSettingsTab::loadSettings() {
    m_extractCheckbox->setChecked(Settings::instance()->extractDownloadedArchives());
    m_subfoldersCheckbox->setChecked(Settings::instance()->createSubfolderForArchives());
    m_subfoldersCheckbox->setEnabled(m_extractCheckbox->isChecked());
    m_deleteCheckbox->setChecked(Settings::instance()->deleteExtractedArchives());
    m_deleteCheckbox->setEnabled(m_extractCheckbox->isChecked());
}

void ArchiveSettingsTab::saveSettings() {
    Settings::instance()->setExtractDownloadedArchives(m_extractCheckbox->isChecked());
    Settings::instance()->setCreateSubfolderForArchives(m_subfoldersCheckbox->isChecked());
    Settings::instance()->setDeleteExtractedArchives(m_deleteCheckbox->isChecked());
}

void ArchiveSettingsTab::showContextMenu(const QPoint &pos) {
    m_contextMenu->popup(m_view->mapToGlobal(pos), m_removePasswordAction);
}

void ArchiveSettingsTab::onPasswordEditTextChanged(const QString &text) {
    m_passwordButton->setEnabled(!text.isEmpty());
}

void ArchiveSettingsTab::addArchivePassword() {
    m_model->addPassword(m_passwordEdit->text());
    m_passwordEdit->clear();
}

void ArchiveSettingsTab::removeArchivePassword() {
    if (m_view->currentIndex().isValid()) {
        m_model->removePassword(m_view->currentIndex().row());
    }
}
