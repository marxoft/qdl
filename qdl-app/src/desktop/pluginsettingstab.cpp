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

#include "pluginsettingstab.h"
#include "separatorlabel.h"
#include "pluginsettingscombobox.h"
#include "pluginsettingscheckbox.h"
#include "pluginsettingsspinbox.h"
#include "pluginsettingslineedit.h"
#include "../shared/pluginsettingsmodel.h"
#include <QFile>
#include <QListView>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QStackedWidget>
#include <QXmlStreamReader>

PluginSettingsTab::PluginSettingsTab(QWidget *parent) :
    QWidget(parent),
    m_model(new PluginSettingsModel(this)),
    m_view(new QListView(this)),
    m_stack(new QStackedWidget(this))
{
    m_view->setModel(m_model);
    m_view->setMaximumWidth(200);
    m_view->setFocus(Qt::OtherFocusReason);

    m_stack->setFrameStyle(QFrame::NoFrame);

    for (int i = 0; i < m_model->rowCount(); i++) {
        QScrollArea *scrollArea = new QScrollArea(this);
        QWidget *scrollWidget = new QWidget(scrollArea);
        QVBoxLayout *vbox = new QVBoxLayout(scrollWidget);
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(scrollWidget);

        QFile file(m_model->data(m_model->index(i), PluginSettingsModel::FileNameRole).toString());
        file.open(QIODevice::ReadOnly);
        QXmlStreamReader reader;
        reader.setDevice(&file);
        reader.readNextStartElement();
        QString title = reader.attributes().value("title").toString();

        while (!reader.atEnd()) {
            if (!reader.attributes().isEmpty()) {
                if (reader.name() == "group") {
                    vbox->addWidget(new SeparatorLabel(reader.attributes().value("title").toString(), this));
                }
                else if (reader.name() == "list") {
                    QLabel *label = new QLabel(reader.attributes().value("title").toString() + ":", this);
                    PluginSettingsCombobox *combobox = new PluginSettingsCombobox(this);
                    combobox->setKey(QString("%1/%2").arg(title).arg(reader.attributes().value("key").toString()));
                    combobox->setDefaultValue(reader.attributes().value("default").toString());
                    reader.readNextStartElement();

                    while (reader.name() == "element") {
                        if (!reader.attributes().isEmpty()) {
                            combobox->addItem(reader.attributes().value("name").toString(), reader.attributes().value("value").toString());
                        }

                        reader.readNextStartElement();
                    }

                    combobox->load();

                    vbox->addWidget(label);
                    vbox->addWidget(combobox);
                }
                else if (reader.name() == "boolean") {
                    PluginSettingsCheckbox *checkbox = new PluginSettingsCheckbox(this);
                    checkbox->setText(reader.attributes().value("title").toString());
                    checkbox->setKey(QString("%1/%2").arg(title).arg(reader.attributes().value("key").toString()));
                    checkbox->setDefaultValue(reader.attributes().value("default").toString());
                    checkbox->load();
                    vbox->addWidget(checkbox);
                }
                else if (reader.name() == "integer") {
                    PluginSettingsSpinbox *spinbox = new PluginSettingsSpinbox(this);
                    spinbox->setKey(QString("%1/%2").arg(title).arg(reader.attributes().value("key").toString()));
                    spinbox->setDefaultValue(reader.attributes().value("default").toString());
                    spinbox->setMinimum(reader.attributes().value("min").toString().toInt());
                    spinbox->setMaximum(reader.attributes().value("max").toString().toInt());
                    spinbox->setSingleStep(reader.attributes().value("step").toString().toInt());
                    spinbox->load();
                    vbox->addWidget(new QLabel(reader.attributes().value("title").toString() + ":", this));
                    vbox->addWidget(spinbox);
                }
                else if (reader.name() == "text") {
                    PluginSettingsLineEdit *lineEdit = new PluginSettingsLineEdit(this);
                    lineEdit->setKey(QString("%1/%2").arg(title).arg(reader.attributes().value("key").toString()));
                    lineEdit->setDefaultValue(reader.attributes().value("default").toString());
                    lineEdit->load();
                    vbox->addWidget(new QLabel(reader.attributes().value("title").toString() + ":", this));
                    vbox->addWidget(lineEdit);
                }
            }

            reader.readNextStartElement();
        }

        vbox->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));

        file.close();

        m_stack->addWidget(scrollArea);
    }

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->addWidget(m_view);
    hbox->addWidget(m_stack);

    this->connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemActivated(QModelIndex)));
    this->connect(m_view, SIGNAL(activated(QModelIndex)), this, SLOT(onItemActivated(QModelIndex)));
}

PluginSettingsTab::~PluginSettingsTab() {}

void PluginSettingsTab::onItemActivated(const QModelIndex &index) {
    m_stack->setCurrentIndex(index.row());
}
