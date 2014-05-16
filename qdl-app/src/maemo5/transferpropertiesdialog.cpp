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

#include "transferpropertiesdialog.h"
#include "valueselector.h"
#include "../shared/utils.h"
#include "../shared/definitions.h"
#include "../shared/selectionmodels.h"
#include "../shared/database.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QLineEdit>

TransferPropertiesDialog::TransferPropertiesDialog(Transfer *transfer, QWidget *parent) :
    QDialog(parent),
    m_transfer(transfer),
    m_progressLabel(new QLabel(this)),
    m_statusLabel(new QLabel(this)),
    m_convertToAudioCheckBox(new QCheckBox(tr("Convert to audio file"), this)),
    m_connectionsSelector(new ValueSelector(tr("Connections"), this)),
    m_categorySelector(new ValueSelector(tr("Category"), this)),
    m_prioritySelector(new ValueSelector(tr("Priority"), this)),
    m_progressBar(new QProgressBar(this)),
    m_startButton(new QPushButton(tr("Start"), this)),
    m_pauseButton(new QPushButton(tr("Pause"), this)),
    m_removeButton(new QPushButton(tr("Remove"), this))
{
    this->setWindowTitle(tr("Download properties"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    if (!m_transfer) {
        return;
    }

    m_convertToAudioCheckBox->setEnabled(m_transfer->convertibleToAudio());

    SelectionModel *connectionsModel = new SelectionModel(m_connectionsSelector);

    for (int i = 1; i <= m_transfer->maximumConnections(); i++) {
        connectionsModel->addItem(QString::number(i), i);
    }

    m_connectionsSelector->setModel(connectionsModel);
    m_connectionsSelector->setValue(m_transfer->preferredConnections());

    SelectionModel *categoryModel = new SelectionModel(m_categorySelector);

    foreach (QString category, Database::instance()->getCategoryNames()) {
        categoryModel->addItem(category, category);
    }

    m_categorySelector->setModel(categoryModel);
    m_categorySelector->setValue(m_transfer->category());

    m_prioritySelector->setModel(new TransferPriorityModel(this));
    m_prioritySelector->setValue(m_transfer->priority());

    m_statusLabel->setWordWrap(true);

    m_progressLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_startButton->setFixedWidth(150);
    m_pauseButton->setFixedWidth(150);
    m_removeButton->setFixedWidth(150);

    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget *scrollWidget = new QWidget(scrollArea);

    QWidget *nameWidget = new QWidget(this);

    QLabel *serviceIcon = new QLabel(nameWidget);
    serviceIcon->setFixedSize(48, 48);
    serviceIcon->setScaledContents(true);
    serviceIcon->setPixmap(QPixmap(m_transfer->iconFileName()));

    QLabel *nameLabel = new QLabel(m_transfer->fileName(), nameWidget);
    nameLabel->setWordWrap(true);

    QHBoxLayout *hbox = new QHBoxLayout(nameWidget);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->addWidget(serviceIcon);
    hbox->addWidget(nameLabel);

    QLineEdit *urlEdit = new QLineEdit(m_transfer->url().toString(), this);
    urlEdit->setReadOnly(true);
    urlEdit->setCursorPosition(0);

    QVBoxLayout *vbox = new QVBoxLayout(scrollWidget);
    vbox->addWidget(nameWidget);
    vbox->addWidget(urlEdit);
    vbox->addWidget(m_convertToAudioCheckBox);
    vbox->addWidget(m_connectionsSelector);
    vbox->addWidget(m_categorySelector);
    vbox->addWidget(m_prioritySelector);
    vbox->addWidget(m_progressLabel);
    vbox->addWidget(m_progressBar);
    vbox->addWidget(m_statusLabel);

    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, this);
    buttonBox->setFixedWidth(150);
    buttonBox->addButton(m_startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_pauseButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_removeButton, QDialogButtonBox::ActionRole);

    QHBoxLayout *hbox2 = new QHBoxLayout(this);
    hbox2->addWidget(scrollArea);
    hbox2->addWidget(buttonBox);

    this->onTransferStatusChanged(m_transfer->status());
    this->onTransferDataChanged(Transfer::SizeRole);
    this->onTransferDataChanged(Transfer::ProgressRole);

    this->connect(m_transfer, SIGNAL(statusChanged(Transfers::Status)), this, SLOT(onTransferStatusChanged(Transfers::Status)));
    this->connect(m_transfer, SIGNAL(dataChanged(int)), this, SLOT(onTransferDataChanged(int)));
    this->connect(m_convertToAudioCheckBox, SIGNAL(clicked(bool)), this, SLOT(setTransferConvertToAudio(bool)));
    this->connect(m_connectionsSelector, SIGNAL(valueChanged(QVariant)), this, SLOT(setTransferConnections(QVariant)));
    this->connect(m_categorySelector, SIGNAL(valueChanged(QVariant)), this, SLOT(setTransferCategory(QVariant)));
    this->connect(m_prioritySelector, SIGNAL(valueChanged(QVariant)), this, SLOT(setTransferPriority(QVariant)));
    this->connect(m_startButton, SIGNAL(clicked()), m_transfer, SLOT(queue()));
    this->connect(m_pauseButton, SIGNAL(clicked()), m_transfer, SLOT(pause()));
    this->connect(m_removeButton, SIGNAL(clicked()), m_transfer, SLOT(cancel()));
}

TransferPropertiesDialog::~TransferPropertiesDialog() {}

void TransferPropertiesDialog::setTransferConnections(const QVariant &value) {
    if (m_transfer) {
        m_transfer->setPreferredConnections(value.toInt());
    }
}

void TransferPropertiesDialog::setTransferCategory(const QVariant &value) {
    if (m_transfer) {
        m_transfer->setCategory(value.toString());
    }
}

void TransferPropertiesDialog::setTransferPriority(const QVariant &value) {
    if (m_transfer) {
        m_transfer->setPriority(static_cast<Transfers::Priority>(value.toInt()));
    }
}

void TransferPropertiesDialog::setTransferConvertToAudio(bool convert) {
    if (m_transfer) {
        m_transfer->setConvertToAudio(convert);
    }
}

void TransferPropertiesDialog::onTransferStatusChanged(Transfers::Status status) {
    if (!m_transfer) {
        return;
    }

    m_statusLabel->setText(m_transfer->statusString());

    switch (status) {
    case Transfers::Paused:
    case Transfers::Failed:
        m_startButton->setEnabled(true);
        m_pauseButton->setEnabled(false);
        return;
    case Transfers::Completed:
    case Transfers::Cancelled:
        this->close();
        return;
    default:
        m_startButton->setEnabled(false);
        m_pauseButton->setEnabled(true);
        return;
    }
}

void TransferPropertiesDialog::onTransferDataChanged(int role) {
    if (!m_transfer) {
        return;
    }

    switch (role) {
    case Transfer::PositionRole:
    case Transfer::ProgressRole:
    case Transfer::SizeRole:
        m_progressBar->setValue(m_transfer->progress());
        m_progressLabel->setText(m_transfer->size() > 0 ? QString("%1 of %2").arg(Utils::fileSizeFromBytes(m_transfer->position())).arg(Utils::fileSizeFromBytes(m_transfer->size()))
                                                        : QString());
        return;
    case Transfer::StatusRole:
        m_statusLabel->setText(m_transfer->statusString());
        return;
    case Transfer::ConvertToAudioRole:
        m_convertToAudioCheckBox->setChecked(m_transfer->convertToAudio());
        return;
    case Transfer::PreferredConnectionsRole:
        m_connectionsSelector->setValue(m_transfer->preferredConnections());
        return;
    case Transfer::CategoryRole:
        m_categorySelector->setValue(m_transfer->category());
        return;
    case Transfer::PriorityRole:
        m_prioritySelector->setValue(m_transfer->priority());
        return;
    default:
        return;
    }
}
