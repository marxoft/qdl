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

#include "settingsdialog.h"
#include "downloadsettingstab.h"
#include "archivesettingstab.h"
#include "proxysettingstab.h"
#include "interfacesettingstab.h"
#include "categorysettingstab.h"
#include "accountsettingstab.h"
#include "captchasettingstab.h"
#include "pluginsettingstab.h"
#include "../shared/pluginsettingsmodel.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QPushButton>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle(tr("Settings"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    DownloadSettingsTab *downloadsTab = new DownloadSettingsTab(this);
    ArchiveSettingsTab *archiveTab = new ArchiveSettingsTab(this);
    ProxySettingsTab *proxyTab = new ProxySettingsTab(this);
    InterfaceSettingsTab *interfaceTab = new InterfaceSettingsTab(this);
    CategorySettingsTab *categoriesTab = new CategorySettingsTab(this);
    AccountSettingsTab *accountsTab = new AccountSettingsTab(this);
    CaptchaSettingsTab *captchaTab = new CaptchaSettingsTab(this);
    PluginSettingsTab *pluginsTab = new PluginSettingsTab(this);

    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->addTab(downloadsTab, tr("General"));
    tabWidget->addTab(archiveTab, tr("Archives"));
    tabWidget->addTab(proxyTab, tr("Proxy"));
    tabWidget->addTab(interfaceTab, tr("Interfaces"));
    tabWidget->addTab(categoriesTab, tr("Categories"));
    tabWidget->addTab(accountsTab, tr("Accounts"));
    tabWidget->addTab(captchaTab, tr("Decaptcha"));
    tabWidget->addTab(pluginsTab, tr("Plugins"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(tabWidget);
    vbox->addWidget(buttonBox);

    this->connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    this->connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    this->connect(buttonBox, SIGNAL(accepted()), downloadsTab, SLOT(saveSettings()));
    this->connect(buttonBox, SIGNAL(accepted()), archiveTab, SLOT(saveSettings()));
    this->connect(buttonBox, SIGNAL(accepted()), proxyTab, SLOT(saveSettings()));
    this->connect(buttonBox, SIGNAL(accepted()), interfaceTab, SLOT(saveSettings()));
}

SettingsDialog::~SettingsDialog() {}
