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

#include "valueselector.h"
#include "../shared/selectionmodels.h"
#include <QMaemo5ListPickSelector>

ValueSelector::ValueSelector(const QString &text, QWidget *parent) :
    QMaemo5ValueButton(text, parent),
    m_model(0),
    m_selector(new QMaemo5ListPickSelector(this))
{
    this->setPickSelector(m_selector);
    this->connect(m_selector, SIGNAL(selected(QString)), this, SLOT(onSelected()));
}

ValueSelector::~ValueSelector() {}

SelectionModel* ValueSelector::model() const {
    return m_model;
}

void ValueSelector::setModel(SelectionModel *model) {
    m_model = model;
    m_selector->setModel(model);
    m_selector->setModelColumn(0);
}

void ValueSelector::setValue(const QVariant &value) {
    if (!m_model) {
        return;
    }

    bool found = false;
    int i = 0;

    while ((!found) && (i < m_model->rowCount())) {
        found = m_model->value(i) == value;

        if (found) {
            m_selector->setCurrentIndex(i);
        }

        i++;
    }

    if (!found) {
        m_selector->setCurrentIndex(0);
    }
}

QVariant ValueSelector::currentValue() {
    return !m_model ? QVariant() : m_model->value(m_selector->currentIndex());
}

void ValueSelector::onSelected() {
    emit valueChanged(this->currentValue());
}
