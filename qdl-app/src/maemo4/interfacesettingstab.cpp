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

#include "interfacesettingstab.h"
#include "../shared/settings.h"
#include "../webif/webinterfacethememodel.h"
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>

InterfaceSettingsTab::InterfaceSettingsTab(QWidget *parent) :
    QWidget(parent),
    m_webIfCheckbox(new QCheckBox(tr("Enable web interface"), this)),
    m_webIfPortSelector(new QSpinBox(this)),
    m_webIfThemeSelector(new QComboBox(this))
{
    m_webIfPortSelector->setMaximum(9999);
    m_webIfThemeSelector->setModel(new WebInterfaceThemeModel(m_webIfThemeSelector));

    QLabel *portLabel = new QLabel(tr("Listen on port") + ":", this);
    portLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    QLabel *themeLabel = new QLabel(tr("Theme") + ":", this);
    themeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_webIfCheckbox, 0, 0, 1, 2);
    grid->addWidget(portLabel, 1, 0);
    grid->addWidget(m_webIfPortSelector, 1, 1);
    grid->addWidget(themeLabel, 2, 0);
    grid->addWidget(m_webIfThemeSelector, 2, 1);
    grid->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding), 3, 0, 1, 2);

    this->connect(m_webIfCheckbox, SIGNAL(toggled(bool)), portLabel, SLOT(setDisabled(bool)));
    this->connect(m_webIfCheckbox, SIGNAL(toggled(bool)), m_webIfPortSelector, SLOT(setDisabled(bool)));

    this->loadSettings();
}

InterfaceSettingsTab::~InterfaceSettingsTab() {}

void InterfaceSettingsTab::loadSettings() {
    m_webIfCheckbox->setChecked(Settings::instance()->enableWebInterface());
    m_webIfPortSelector->setValue(Settings::instance()->webInterfacePort());
    m_webIfThemeSelector->setCurrentIndex(m_webIfThemeSelector->findText(Settings::instance()->webInterfaceTheme()));
}

void InterfaceSettingsTab::saveSettings() {
    Settings::instance()->setEnableWebInterface(m_webIfCheckbox->isChecked());
    Settings::instance()->setWebInterfacePort(m_webIfPortSelector->value());
    Settings::instance()->setWebInterfaceTheme(m_webIfThemeSelector->currentText());
}
