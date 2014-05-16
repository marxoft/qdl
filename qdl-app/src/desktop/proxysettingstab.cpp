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

#include "proxysettingstab.h"
#include "../shared/settings.h"
#include "../shared/selectionmodels.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>

ProxySettingsTab::ProxySettingsTab(QWidget *parent) :
    QWidget(parent),
    m_proxyCheckbox(new QCheckBox(tr("Use network proxy"), this)),
    m_proxyTypeCombobox(new QComboBox(this)),
    m_proxyHostEdit(new QLineEdit(this)),
    m_proxyPortEdit(new QLineEdit(this)),
    m_proxyUserEdit(new QLineEdit(this)),
    m_proxyPassEdit(new QLineEdit(this))
{
    QFrame *proxyFrame = new QFrame(this);
    proxyFrame->setFrameStyle(QFrame::Raised);
    proxyFrame->setEnabled(!Settings::instance()->networkProxyHostName().isEmpty());

    QGridLayout *proxyGrid = new QGridLayout(proxyFrame);
    proxyGrid->setContentsMargins(0, 0, 0, 0);
    proxyGrid->addWidget(new QLabel(tr("Proxy type") + ":", this), 0, 0);
    proxyGrid->addWidget(m_proxyTypeCombobox, 0, 1);
    proxyGrid->addWidget(new QLabel(tr("Host") + ":", this), 1, 0);
    proxyGrid->addWidget(m_proxyHostEdit, 1, 1);
    proxyGrid->addWidget(new QLabel(tr("Port") + ":", this), 1, 2);
    proxyGrid->addWidget(m_proxyPortEdit, 1, 3);
    proxyGrid->addWidget(new QLabel(tr("Username") + ":", this), 2, 0);
    proxyGrid->addWidget(m_proxyUserEdit, 2, 1);
    proxyGrid->addWidget(new QLabel(tr("Password") + ":", this), 2, 2);
    proxyGrid->addWidget(m_proxyPassEdit, 2, 3);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(m_proxyCheckbox);
    vbox->addWidget(proxyFrame);
    vbox->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));

    m_proxyPassEdit->setEchoMode(QLineEdit::Password);

    this->connect(m_proxyCheckbox, SIGNAL(toggled(bool)), proxyFrame, SLOT(setEnabled(bool)));
    this->connect(m_proxyCheckbox, SIGNAL(toggled(bool)), this, SLOT(onProxyCheckboxToggled(bool)));

    this->loadSettings();
}

ProxySettingsTab::~ProxySettingsTab() {}

void ProxySettingsTab::loadSettings() {
    m_proxyCheckbox->setChecked(!Settings::instance()->networkProxyHostName().isEmpty());
    m_proxyTypeCombobox->setModel(new NetworkProxyTypeModel(this));
    m_proxyTypeCombobox->setCurrentIndex(m_proxyTypeCombobox->findData(Settings::instance()->networkProxyType(), Qt::UserRole + 1));
    m_proxyHostEdit->setText(Settings::instance()->networkProxyHostName());
    m_proxyPortEdit->setText(QString::number(Settings::instance()->networkProxyPort()));
    m_proxyPortEdit->setValidator(new QIntValidator(0, 100000, this));
    m_proxyUserEdit->setText(Settings::instance()->networkProxyUser());
    m_proxyPassEdit->setText(Settings::instance()->networkProxyPassword());
}

void ProxySettingsTab::saveSettings() {
    Settings::instance()->setNetworkProxyType(static_cast<NetworkProxyType::ProxyType>(m_proxyTypeCombobox->itemData(m_proxyTypeCombobox->currentIndex(), Qt::UserRole + 1).toInt()));
    Settings::instance()->setNetworkProxyHostName(m_proxyHostEdit->text());
    Settings::instance()->setNetworkProxyPort(m_proxyPortEdit->text().toUInt());
    Settings::instance()->setNetworkProxyUser(m_proxyUserEdit->text());
    Settings::instance()->setNetworkProxyPassword(m_proxyPassEdit->text());
    Settings::instance()->setNetworkProxy();
}

void ProxySettingsTab::onProxyCheckboxToggled(bool checked) {
    if (!checked) {
        m_proxyHostEdit->clear();
        m_proxyUserEdit->clear();
        m_proxyPassEdit->clear();
    }
}
