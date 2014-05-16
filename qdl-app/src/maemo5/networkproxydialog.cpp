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

#include "networkproxydialog.h"
#include "valueselector.h"
#include "../shared/selectionmodels.h"
#include "../shared/settings.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QDialogButtonBox>
#include <QIntValidator>

NetworkProxyDialog::NetworkProxyDialog(QWidget *parent) :
    QDialog(parent),
    m_proxyCheckbox(new QCheckBox(tr("Use network proxy"), this)),
    m_proxyWidget(new QWidget(this)),
    m_proxyTypeSelector(new ValueSelector(tr("Proxy type"), this)),
    m_hostEdit(new QLineEdit(this)),
    m_portEdit(new QLineEdit(this)),
    m_userEdit(new QLineEdit(this)),
    m_passEdit(new QLineEdit(this))
{
    this->setWindowTitle(tr("Network proxy"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    m_proxyTypeSelector->setModel(new NetworkProxyTypeModel(this));

    m_hostEdit->setMinimumWidth(380);
    m_portEdit->setValidator(new QIntValidator(0, 100000, this));
    m_passEdit->setEchoMode(QLineEdit::Password);

    QGridLayout *proxyGrid = new QGridLayout(m_proxyWidget);
    proxyGrid->setContentsMargins(0, 0, 0, 0);
    proxyGrid->addWidget(m_proxyTypeSelector, 0, 0, 1, 2);
    proxyGrid->addWidget(new QLabel(tr("Host"), this), 1, 0, 1, 1);
    proxyGrid->addWidget(new QLabel(tr("Port"), this), 1, 1, 1, 1);
    proxyGrid->addWidget(m_hostEdit, 2, 0, 1, 1);
    proxyGrid->addWidget(m_portEdit, 2, 1, 1, 1);
    proxyGrid->addWidget(new QLabel(tr("Username"), this), 3, 0, 1, 2);
    proxyGrid->addWidget(m_userEdit, 4, 0, 1, 2);
    proxyGrid->addWidget(new QLabel(tr("Password"), this), 5, 0, 1, 2);
    proxyGrid->addWidget(m_passEdit, 6, 0, 1, 2);

    QWidget *scrollWidget = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout(scrollWidget);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(m_proxyCheckbox);
    vbox->addWidget(m_proxyWidget);
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidget(scrollWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save, Qt::Vertical, this);
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->addWidget(scrollArea);
    hbox->addWidget(buttonBox);

    this->connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
    this->connect(m_proxyCheckbox, SIGNAL(toggled(bool)), m_proxyWidget, SLOT(setEnabled(bool)));
    this->connect(m_proxyCheckbox, SIGNAL(toggled(bool)), this, SLOT(onProxyCheckboxToggled(bool)));

    this->loadSettings();
}

NetworkProxyDialog::~NetworkProxyDialog() {}

void NetworkProxyDialog::loadSettings() {
    m_proxyCheckbox->setChecked(!Settings::instance()->networkProxyHostName().isEmpty());
    m_proxyTypeSelector->setValue(Settings::instance()->networkProxyType());
    m_hostEdit->setText(Settings::instance()->networkProxyHostName());
    m_portEdit->setText(QString::number(Settings::instance()->networkProxyPort()));
    m_userEdit->setText(Settings::instance()->networkProxyUser());
    m_passEdit->setText(Settings::instance()->networkProxyPassword());
    m_proxyWidget->setEnabled(m_proxyCheckbox->isChecked());
}

void NetworkProxyDialog::saveSettings() {
    Settings::instance()->setNetworkProxyType(static_cast<NetworkProxyType::ProxyType>(m_proxyTypeSelector->currentValue().toInt()));
    Settings::instance()->setNetworkProxyHostName(m_hostEdit->text());
    Settings::instance()->setNetworkProxyPort(m_portEdit->text().toUInt());
    Settings::instance()->setNetworkProxyUser(m_userEdit->text());
    Settings::instance()->setNetworkProxyPassword(m_passEdit->text());
    Settings::instance()->setNetworkProxy();
    this->accept();
}

void NetworkProxyDialog::onProxyCheckboxToggled(bool checked) {
    if (!checked) {
        m_hostEdit->clear();
        m_portEdit->setText(QString::number(80));
        m_userEdit->clear();
        m_passEdit->clear();
    }
}
