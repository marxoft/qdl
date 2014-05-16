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

#include "pluginsettingscombobox.h"
#include <QSettings>

PluginSettingsCombobox::PluginSettingsCombobox(QWidget *parent) :
    QComboBox(parent)
{
}

PluginSettingsCombobox::~PluginSettingsCombobox() {}

void PluginSettingsCombobox::setKey(const QString &key) {
    m_key = key;
}

void PluginSettingsCombobox::setDefaultValue(const QVariant &value) {
    m_default = value;
}

void PluginSettingsCombobox::load() {
    QVariant value = QSettings("QDL", "QDL").value(this->key(), this->defaultValue());

    bool found = false;
    int i = 0;

    while ((!found) && (i < this->count())) {
        found = this->itemData(i) == value;

        if (found) {
            this->setCurrentIndex(i);
        }

        i++;
    }

    if (!found) {
        this->setCurrentIndex(0);
    }

    this->connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
}

void PluginSettingsCombobox::onCurrentIndexChanged(int index) {
    if (!this->key().isEmpty()) {
        QSettings("QDL", "QDL").setValue(this->key(), this->itemData(index));
    }
}
