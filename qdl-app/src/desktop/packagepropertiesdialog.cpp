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
#include "../shared/packagetransfermodel.h"
#include "../shared/selectionmodels.h"
#include "../shared/database.h"
#include <QLabel>
#include <QGridLayout>
#include <QComboBox>
#include <QListView>
#include <QPushButton>
#include <QDialogButtonBox>

PackagePropertiesDialog::PackagePropertiesDialog(Transfer *package, QWidget *parent) :
    QDialog(parent),
    m_package(package),
    m_model(new PackageTransferModel(this, package)),
    m_view(new QListView(this)),
    m_categorySelector(new QComboBox(this)),
    m_prioritySelector(new QComboBox(this)),
    m_startButton(new QPushButton(QIcon::fromTheme("media-playback-start"), tr("Start"), this)),
    m_pauseButton(new QPushButton(QIcon::fromTheme("media-playback-pause"), tr("Pause"), this)),
    m_removeButton(new QPushButton(QIcon::fromTheme("edit-delete"), tr("Remove"), this))
{
    this->setWindowTitle(tr("Package properties"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setMinimumSize(400, 400);

    if (!m_package) {
        return;
    }

    m_view->setModel(m_model);
    m_view->setItemDelegate(new PackageTransferDelegate(m_view));
    m_view->setMinimumHeight(!m_model->rowCount() ? 60 : m_view->sizeHintForRow(0) * 3);
    m_view->setEditTriggers(QListView::NoEditTriggers);

    m_prioritySelector->setModel(new TransferPriorityModel(m_prioritySelector));
    m_prioritySelector->setCurrentIndex(Transfers::Priority(m_package->priority()));

    foreach (QString category, Database::instance()->getCategoryNames()) {
        m_categorySelector->addItem(category, category);
    }

    m_categorySelector->setCurrentIndex(m_categorySelector->findText(m_package->category()));
    m_categorySelector->setEnabled(m_categorySelector->count() > 0);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    buttonBox->button(QDialogButtonBox::Close)->setIcon(QIcon::fromTheme("dialog-cancel"));

    QGridLayout *grid = new QGridLayout(this);

    grid->addWidget(new QLabel(QString("<b>%1</b>").arg(m_package->packageName()), this), 0, 0, 1, 3);
    grid->addWidget(new QLabel(tr("Category") + ":", this), 2, 0);
    grid->addWidget(m_categorySelector, 2, 1, 1, 2);
    grid->addWidget(new QLabel(tr("Priority") + ":", this), 3, 0);
    grid->addWidget(m_prioritySelector, 3, 1, 1, 2);
    grid->addWidget(new QLabel(tr("Files") + ":", this), 4, 0, 1, 3);
    grid->addWidget(m_view, 5, 0, 1, 3);
    grid->addWidget(m_startButton, 6, 0);
    grid->addWidget(m_pauseButton, 6, 1);
    grid->addWidget(m_removeButton, 6, 2);
    grid->addWidget(buttonBox, 7, 0, 1, 3);

    this->connect(m_package, SIGNAL(dataChanged(int)), this, SLOT(onPackageDataChanged(int)));
    this->connect(m_model, SIGNAL(countChanged(int)), this, SLOT(onTransferCountChanged(int)));
    this->connect(m_categorySelector, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPackageCategory(QString)));
    this->connect(m_prioritySelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setPackagePriority(int)));
    this->connect(m_startButton, SIGNAL(clicked()), m_package, SLOT(queuePackage()));
    this->connect(m_pauseButton, SIGNAL(clicked()), m_package, SLOT(pausePackage()));
    this->connect(m_removeButton, SIGNAL(clicked()), m_package, SLOT(cancelPackage()));
    this->connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    this->connect(buttonBox, SIGNAL(accepted()), this, SLOT(close()));
}

PackagePropertiesDialog::~PackagePropertiesDialog() {}

void PackagePropertiesDialog::setPackageCategory(const QString &text) {
    if (m_package) {
        m_package->setCategory(text);
    }
}

void PackagePropertiesDialog::setPackagePriority(int index) {
    if (m_package) {
        m_package->setPriority(static_cast<Transfers::Priority>(index));
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
        m_categorySelector->setCurrentIndex(m_categorySelector->findText(m_package->category()));
        return;
    case Transfer::PriorityRole:
        m_prioritySelector->setCurrentIndex(Transfers::Priority(m_package->priority()));
        return;
    default:
        return;
    }
}
