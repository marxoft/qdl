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

#include "valuedialog.h"
#include "../shared/selectionmodels.h"
#include <QListView>
#include <QVBoxLayout>

ValueDialog::ValueDialog(QWidget *parent) :
    QDialog(parent),
    m_model(0),
    m_view(new QListView(this))
{
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(m_view);

    this->connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
}

ValueDialog::~ValueDialog() {}

SelectionModel* ValueDialog::model() const {
    return m_model;
}

void ValueDialog::setModel(SelectionModel *model) {
    m_model = model;
    m_view->setModel(model);

    if (model->rowCount() > 0) {
        m_view->setMinimumHeight(m_view->sizeHintForRow(0) * 5);
    }
}

void ValueDialog::setValue(const QVariant &value) {
    if (!m_model) {
        return;
    }

    bool found = false;
    int i = 0;

    while ((!found) && (i < m_model->rowCount())) {
        found = m_model->value(i) == value;

        if (found) {
            m_view->setCurrentIndex(m_model->index(i, 0));
        }

        i++;
    }

    if (!found) {
        m_view->setCurrentIndex(m_model->index(0, 0));
    }
}

QVariant ValueDialog::currentValue() {
    return !m_model ? QVariant() : m_model->value(m_view->currentIndex().row());
}

void ValueDialog::onItemClicked(const QModelIndex &index) {
    emit valueChanged(index.data(Qt::UserRole + 1));
}
