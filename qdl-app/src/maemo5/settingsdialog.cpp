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
#include "archivepasswordsdialog.h"
#include "categoriesdialog.h"
#include "serviceaccountsdialog.h"
#include "decaptchaaccountsdialog.h"
#include "separatorlabel.h"
#include "listview.h"
#include "pluginsettingsdialog.h"
#include "networkproxydialog.h"
#include "valueselector.h"
#include "../shared/pluginsettingsmodel.h"
#include "../shared/settings.h"
#include "../shared/transfermodel.h"
#include "../webif/webinterfacethememodel.h"
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QFileDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QMaemo5ValueButton>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_pathSelector(new QMaemo5ValueButton(tr("Default download path"), this)),
    m_statusCheckbox(new QCheckBox(tr("Start downloads automatically"), this)),
    m_clipboardCheckbox(new QCheckBox(tr("Monitor clipboard for URLs"), this)),
    m_extractArchivesCheckbox(new QCheckBox(tr("Extract downloaded archives"), this)),
    m_archiveSubfoldersCheckbox(new QCheckBox(tr("Create subfolders for archives"), this)),
    m_deleteArchivesCheckbox(new QCheckBox(tr("Delete extracted archives"), this)),
    m_webIfCheckbox(new QCheckBox(tr("Enable web interface"), this)),
    m_webIfPortSelector(new QSpinBox(this)),
    m_webIfThemeSelector(new ValueSelector(tr("Theme")))
{
    this->setWindowTitle(tr("Settings"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    m_webIfPortSelector->setMaximum(9999);
    m_webIfThemeSelector->setModel(new WebInterfaceThemeModel(m_webIfThemeSelector));

    QPushButton *archivePasswordsButton = new QPushButton(tr("Archive passwords"), this);
    QPushButton *categoriesButton = new QPushButton(tr("Categories"), this);
    QPushButton *servicesButton = new QPushButton(tr("Service accounts"), this);
    QPushButton *decaptchaButton = new QPushButton(tr("Decaptcha accounts"), this);
    QPushButton *proxyButton = new QPushButton(tr("Network proxy"), this);

    PluginSettingsModel *model = new PluginSettingsModel(this);
    ListView *view = new ListView(this);
    view->setModel(model);
    view->setFixedHeight(!model->rowCount() ? 0 : model->rowCount() * view->sizeHintForRow(0));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save, Qt::Vertical, this);
    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget *scrollWidget = new QWidget(scrollArea);
    QVBoxLayout *vbox = new QVBoxLayout(scrollWidget);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(new SeparatorLabel(tr("General"), this));
    vbox->addWidget(m_pathSelector);
    vbox->addWidget(m_statusCheckbox);
    vbox->addWidget(m_clipboardCheckbox);
    vbox->addWidget(new SeparatorLabel(tr("Archives"), this));
    vbox->addWidget(m_extractArchivesCheckbox);
    vbox->addWidget(m_archiveSubfoldersCheckbox);
    vbox->addWidget(m_deleteArchivesCheckbox);
    vbox->addWidget(archivePasswordsButton);
    vbox->addWidget(new SeparatorLabel(tr("Network"), this));
    vbox->addWidget(proxyButton);
    vbox->addWidget(new SeparatorLabel(tr("Web interface"), this));
    vbox->addWidget(m_webIfCheckbox);
    vbox->addWidget(new QLabel(tr("Listen on port"), this));
    vbox->addWidget(m_webIfPortSelector);
    vbox->addWidget(m_webIfThemeSelector);
    vbox->addWidget(new SeparatorLabel(tr("Categories"), this));
    vbox->addWidget(categoriesButton);
    vbox->addWidget(new SeparatorLabel(tr("Accounts"), this));
    vbox->addWidget(servicesButton);
    vbox->addWidget(decaptchaButton);
    vbox->addWidget(new SeparatorLabel(tr("Plugins"), this));
    vbox->addWidget(view);

    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->addWidget(scrollArea, 0, Qt::AlignBottom);
    hbox->addWidget(buttonBox, 0, Qt::AlignBottom);

    this->connect(m_pathSelector, SIGNAL(clicked()), this, SLOT(showFileDialog()));
    this->connect(m_extractArchivesCheckbox, SIGNAL(toggled(bool)), m_archiveSubfoldersCheckbox, SLOT(setEnabled(bool)));
    this->connect(m_extractArchivesCheckbox, SIGNAL(toggled(bool)), m_deleteArchivesCheckbox, SLOT(setEnabled(bool)));
    this->connect(m_webIfCheckbox, SIGNAL(toggled(bool)), m_webIfPortSelector, SLOT(setDisabled(bool)));
    this->connect(archivePasswordsButton, SIGNAL(clicked()), this, SLOT(showArchivePasswordsDialog()));
    this->connect(categoriesButton, SIGNAL(clicked()), this, SLOT(showCategoriesDialog()));
    this->connect(servicesButton, SIGNAL(clicked()), this, SLOT(showServiceAccountsDialog()));
    this->connect(decaptchaButton, SIGNAL(clicked()), this, SLOT(showDecaptchaAccountsDialog()));
    this->connect(view, SIGNAL(clicked(QModelIndex)), this, SLOT(showPluginSettingsDialog(QModelIndex)));
    this->connect(proxyButton, SIGNAL(clicked()), this, SLOT(showNetworkProxyDialog()));
    this->connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));

    this->loadSettings();
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::loadSettings() {
    m_pathSelector->setEnabled(TransferModel::instance()->rowCount() == 0);
    m_pathSelector->setValueText(Settings::instance()->downloadPath());
    m_statusCheckbox->setChecked(Settings::instance()->startTransfersAutomatically());
    m_clipboardCheckbox->setChecked(Settings::instance()->monitorClipboard());
    m_extractArchivesCheckbox->setChecked(Settings::instance()->extractDownloadedArchives());
    m_archiveSubfoldersCheckbox->setChecked(Settings::instance()->createSubfolderForArchives());
    m_archiveSubfoldersCheckbox->setEnabled(m_extractArchivesCheckbox->isChecked());
    m_deleteArchivesCheckbox->setChecked(Settings::instance()->deleteExtractedArchives());
    m_deleteArchivesCheckbox->setEnabled(m_extractArchivesCheckbox->isChecked());
    m_webIfCheckbox->setChecked(Settings::instance()->enableWebInterface());
    m_webIfPortSelector->setValue(Settings::instance()->webInterfacePort());
    m_webIfThemeSelector->setValue(Settings::instance()->webInterfaceTheme());
}

void SettingsDialog::saveSettings() {
    Settings::instance()->setDownloadPath(m_pathSelector->valueText());
    Settings::instance()->setStartTransfersAutomatically(m_statusCheckbox->isChecked());
    Settings::instance()->setMonitorClipboard(m_clipboardCheckbox->isChecked());
    Settings::instance()->setExtractDownloadedArchives(m_extractArchivesCheckbox->isChecked());
    Settings::instance()->setCreateSubfolderForArchives(m_archiveSubfoldersCheckbox->isChecked());
    Settings::instance()->setDeleteExtractedArchives(m_deleteArchivesCheckbox->isChecked());
    Settings::instance()->setEnableWebInterface(m_webIfCheckbox->isChecked());
    Settings::instance()->setWebInterfacePort(m_webIfPortSelector->value());
    Settings::instance()->setWebInterfaceTheme(m_webIfThemeSelector->currentValue().toString());

    this->accept();
}

void SettingsDialog::showFileDialog() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose folder"), Settings::instance()->downloadPath());

    if (!path.isEmpty()) {
        m_pathSelector->setValueText(path);
    }
}

void SettingsDialog::showArchivePasswordsDialog() {
    ArchivePasswordsDialog *dialog = new ArchivePasswordsDialog(this);
    dialog->open();
}

void SettingsDialog::showCategoriesDialog() {
    CategoriesDialog *dialog = new CategoriesDialog(this);
    dialog->open();
}

void SettingsDialog::showServiceAccountsDialog() {
    ServiceAccountsDialog *dialog = new ServiceAccountsDialog(this);
    dialog->open();
}

void SettingsDialog::showDecaptchaAccountsDialog() {
    DecaptchaAccountsDialog *dialog = new DecaptchaAccountsDialog(this);
    dialog->open();
}

void SettingsDialog::showPluginSettingsDialog(const QModelIndex &index) {
    QString name = index.data(PluginSettingsModel::PluginNameRole).toString();
    QString fileName = index.data(PluginSettingsModel::FileNameRole).toString();
    PluginSettingsDialog *dialog = new PluginSettingsDialog(name, fileName, this);
    dialog->open();
}

void SettingsDialog::showNetworkProxyDialog() {
    NetworkProxyDialog *dialog = new NetworkProxyDialog(this);
    dialog->open();
}
