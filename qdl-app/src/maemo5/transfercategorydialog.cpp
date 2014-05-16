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

#include "transfercategorydialog.h"
#include "textlistdelegate.h"
#include "../shared/categoriesmodel.h"
#include <QListView>
#include <QGridLayout>

TransferCategoryDialog::TransferCategoryDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new CategoriesModel(this)),
    m_listView(new QListView(this))
{
    this->setWindowTitle(tr("Category"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setFixedHeight(340);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(m_listView, 0, 0);

    m_listView->setModel(m_model);
    m_listView->setItemDelegate(new TextListDelegate(CategoriesModel::NameRole, Qt::AlignCenter, m_listView));

    this->connect(m_listView, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
}

TransferCategoryDialog::~TransferCategoryDialog() {}

void TransferCategoryDialog::setValue(const QString &value) {
    bool found = false;
    int i = 0;

    while ((!found) && (i < m_model->rowCount())) {
        found = m_model->index(i, 0).data(CategoriesModel::NameRole).toString() == value;

        if (found) {
            m_listView->setCurrentIndex(m_model->index(i, 0));
        }

        i++;
    }

    if (!found) {
        m_listView->setCurrentIndex(m_model->index(0, 0));
    }
}

QString TransferCategoryDialog::currentValue() const {
    return m_listView->currentIndex().data(CategoriesModel::NameRole).toString();
}

void TransferCategoryDialog::onItemClicked(const QModelIndex &index) {
    emit valueChanged(index.data(CategoriesModel::NameRole).toString());
    this->accept();
}
