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

#include "valueselectoraction.h"
#include "valueselector.h"
#include "../shared/selectionmodels.h"

ValueSelectorAction::ValueSelectorAction(QWidget *parent) :
    QWidgetAction(parent)
{
}

ValueSelectorAction::~ValueSelectorAction() {}

SelectionModel* ValueSelectorAction::model() const {
    return m_model;
}

void ValueSelectorAction::setModel(SelectionModel *model) {
    m_model = model;
}

QVariant ValueSelectorAction::value() const {
    return m_value;
}

void ValueSelectorAction::setValue(const QVariant &value) {
    if (value != this->value()) {
        m_value = value;
        emit valueChanged(value);
    }
}

QWidget* ValueSelectorAction::createWidget(QWidget *parent) {
    ValueSelector *selector = new ValueSelector(this->text(), parent);

    if (this->model()) {
        selector->setModel(this->model());

        if (this->value().isValid()) {
            selector->setValue(this->value());
        }
    }

    this->connect(selector, SIGNAL(valueChanged(QVariant)), this, SLOT(setValue(QVariant)));

    return selector;
}
