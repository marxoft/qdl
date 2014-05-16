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

#include "packagepropertiesdialog.h"
#include "packagetransferdelegate.h"
#include "valueselector.h"
#include "separatorlabel.h"
#include "../shared/packagetransfermodel.h"
#include "../shared/selectionmodels.h"
#include "../shared/database.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QScrollArea>

PackagePropertiesDialog::PackagePropertiesDialog(Transfer *package, QWidget *parent) :
    QDialog(parent),
    m_package(package),
    m_model(new PackageTransferModel(this, package)),
    m_view(new QListView(this)),
    m_categorySelector(new ValueSelector(tr("Category"), this)),
    m_prioritySelector(new ValueSelector(tr("Priority"), this)),
    m_startButton(new QPushButton(tr("Start"), this)),
    m_pauseButton(new QPushButton(tr("Pause"), this)),
    m_removeButton(new QPushButton(tr("Remove"), this))
{
    this->setWindowTitle(tr("Package properties"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    if (!m_package) {
        return;
    }

    m_view->setModel(m_model);
    m_view->setItemDelegate(new PackageTransferDelegate(m_view));
    m_view->setFixedHeight(!m_model->rowCount() ? 0 : m_view->sizeHintForRow(0) * m_model->rowCount());
    m_view->setEditTriggers(QListView::NoEditTriggers);

    m_prioritySelector->setModel(new TransferPriorityModel(m_prioritySelector));
    m_prioritySelector->setValue(m_package->priority());

    SelectionModel *categoryModel = new SelectionModel(m_categorySelector);

    foreach (QString category, Database::instance()->getCategoryNames()) {
        categoryModel->addItem(category, category);
    }

    m_categorySelector->setModel(categoryModel);
    m_categorySelector->setValue(m_package->category());
    m_categorySelector->setEnabled(categoryModel->rowCount() > 0);

    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget *scrollWidget = new QWidget(scrollArea);

    QLabel *nameLabel = new QLabel(m_package->packageName(), this);
    nameLabel->setWordWrap(true);

    QVBoxLayout *vbox = new QVBoxLayout(scrollWidget);

    vbox->addWidget(nameLabel);
    vbox->addWidget(m_categorySelector);
    vbox->addWidget(m_prioritySelector);
    vbox->addWidget(new SeparatorLabel(tr("Files"), this));
    vbox->addWidget(m_view);

    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, this);
    buttonBox->setFixedWidth(150);
    buttonBox->addButton(m_startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_pauseButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_removeButton, QDialogButtonBox::ActionRole);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->addWidget(scrollArea);
    hbox->addWidget(buttonBox);

    this->connect(m_package, SIGNAL(dataChanged(int)), this, SLOT(onPackageDataChanged(int)));
    this->connect(m_model, SIGNAL(countChanged(int)), this, SLOT(onTransferCountChanged(int)));
    this->connect(m_categorySelector, SIGNAL(valueChanged(QVariant)), this, SLOT(setPackageCategory(QVariant)));
    this->connect(m_prioritySelector, SIGNAL(valueChanged(QVariant)), this, SLOT(setPackagePriority(QVariant)));
    this->connect(m_startButton, SIGNAL(clicked()), m_package, SLOT(queuePackage()));
    this->connect(m_pauseButton, SIGNAL(clicked()), m_package, SLOT(pausePackage()));
    this->connect(m_removeButton, SIGNAL(clicked()), m_package, SLOT(cancelPackage()));
}

PackagePropertiesDialog::~PackagePropertiesDialog() {}

void PackagePropertiesDialog::setPackageCategory(const QVariant &value) {
    if (m_package) {
        m_package->setCategory(value.toString());
    }
}

void PackagePropertiesDialog::setPackagePriority(const QVariant &value) {
    if (m_package) {
        m_package->setPriority(static_cast<Transfers::Priority>(value.toInt()));
    }
}

void PackagePropertiesDialog::onTransferCountChanged(int count) {
    if (count > 0) {
        m_package = m_model->package();

        if (m_package) {
            this->connect(m_package, SIGNAL(dataChanged(int)), this, SLOT(onPackageDataChanged(int)));
        }
    }
    else {
        this->close();
    }
}

void PackagePropertiesDialog::onPackageDataChanged(int role) {
    if (!m_package) {
        return;
    }

    switch (role) {
    case Transfer::CategoryRole:
        m_categorySelector->setValue(m_package->category());
        return;
    case Transfer::PriorityRole:
        m_prioritySelector->setValue(m_package->priority());
        return;
    default:
        return;
    }
}
