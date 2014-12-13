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
#include "../shared/utils.h"
#include "../shared/selectionmodels.h"
#include "../shared/database.h"
#include <QLabel>
#include <QGridLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QProgressBar>
#include <QScrollArea>
#include <QLineEdit>

TransferPropertiesDialog::TransferPropertiesDialog(Transfer *transfer, QWidget *parent) :
    QDialog(parent),
    m_transfer(transfer),
    m_convertToAudioCheckBox(new QCheckBox(tr("Convert to audio file"), this)),
    m_connectionsSelector(new QComboBox(this)),
    m_categorySelector(new QComboBox(this)),
    m_prioritySelector(new QComboBox(this)),
    m_progressLabel(new QLabel(this)),
    m_statusLabel(new QLabel(this)),
    m_progressBar(new QProgressBar(this)),
    m_startButton(new QPushButton(tr("Start"), this)),
    m_pauseButton(new QPushButton(tr("Pause"), this)),
    m_removeButton(new QPushButton(tr("Remove"), this))
{
    this->setWindowTitle(tr("Download properties"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    if (!m_transfer) {
        return;
    }

    m_convertToAudioCheckBox->setEnabled(m_transfer->convertibleToAudio());
    m_convertToAudioCheckBox->setChecked(m_transfer->convertToAudio());

    for (int i = 1; i <= m_transfer->maximumConnections(); i++) {
        m_connectionsSelector->addItem(QString::number(i), i);
    }

    m_connectionsSelector->setCurrentIndex(m_transfer->preferredConnections() - 1);

    m_prioritySelector->setModel(new TransferPriorityModel(m_prioritySelector));
    m_prioritySelector->setCurrentIndex(Transfers::Priority(m_transfer->priority()));

    foreach (QString category, Database::instance()->getCategoryNames()) {
        m_categorySelector->addItem(category, category);
    }

    m_categorySelector->setCurrentIndex(m_categorySelector->findText(m_transfer->category()));
    m_categorySelector->setEnabled(m_categorySelector->count() > 0);

    m_progressLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget *scrollWidget = new QWidget(scrollArea);

    QWidget *nameWidget = new QWidget(this);

    QLabel *serviceIcon = new QLabel(nameWidget);
    serviceIcon->setFixedSize(48, 48);
    serviceIcon->setScaledContents(true);
    serviceIcon->setPixmap(QPixmap(m_transfer->iconFileName()));

    QLineEdit *urlEdit = new QLineEdit(transfer->url().toString(), this);
    urlEdit->setReadOnly(true);
    urlEdit->setCursorPosition(0);

    QHBoxLayout *hbox = new QHBoxLayout(nameWidget);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->addWidget(serviceIcon);
    hbox->addWidget(new QLabel(QString("<b>%1</b>").arg(m_transfer->fileName()), nameWidget));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    buttonBox->addButton(m_startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_pauseButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_removeButton, QDialogButtonBox::ActionRole);

    QGridLayout *grid = new QGridLayout(scrollWidget);
    grid->addWidget(nameWidget, 0, 0, 1, 2);
    grid->addWidget(urlEdit, 1, 0, 1, 2);
    grid->addWidget(m_convertToAudioCheckBox, 2, 0, 1, 2);
    grid->addWidget(new QLabel(tr("Connections") + ":", this), 3, 0);
    grid->addWidget(m_connectionsSelector, 3, 1);
    grid->addWidget(new QLabel(tr("Category") + ":", this), 4, 0);
    grid->addWidget(m_categorySelector, 4, 1);
    grid->addWidget(new QLabel(tr("Priority") + ":", this), 5, 0);
    grid->addWidget(m_prioritySelector, 5, 1);
    grid->addWidget(m_progressLabel, 6, 0, 1, 2);
    grid->addWidget(m_progressBar, 7, 0, 1, 2);
    grid->addWidget(m_statusLabel, 8, 0, 1, 2);

    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(scrollArea);
    vbox->addWidget(buttonBox);

    this->onTransferStatusChanged(m_transfer->status());
    this->onTransferDataChanged(Transfer::SizeRole);
    this->onTransferDataChanged(Transfer::ProgressRole);

    this->connect(m_transfer, SIGNAL(statusChanged(Transfers::Status)), this, SLOT(onTransferStatusChanged(Transfers::Status)));
    this->connect(m_transfer, SIGNAL(dataChanged(int)), this, SLOT(onTransferDataChanged(int)));
    this->connect(m_convertToAudioCheckBox, SIGNAL(clicked(bool)), this, SLOT(setTransferConvertToAudio(bool)));
    this->connect(m_connectionsSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setTransferConnections(int)));
    this->connect(m_categorySelector, SIGNAL(currentIndexChanged(QString)), this, SLOT(setTransferCategory(QString)));
    this->connect(m_prioritySelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setTransferPriority(int)));
    this->connect(m_startButton, SIGNAL(clicked()), m_transfer, SLOT(queue()));
    this->connect(m_pauseButton, SIGNAL(clicked()), m_transfer, SLOT(pause()));
    this->connect(m_removeButton, SIGNAL(clicked()), m_transfer, SLOT(cancel()));
    this->connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    this->connect(buttonBox, SIGNAL(accepted()), this, SLOT(close()));
}

TransferPropertiesDialog::~TransferPropertiesDialog() {}

void TransferPropertiesDialog::setTransferConnections(int index) {
    if (m_transfer) {
        m_transfer->setPreferredConnections(index + 1);
    }
}

void TransferPropertiesDialog::setTransferCategory(const QString &text) {
    if (m_transfer) {
        m_transfer->setCategory(text);
    }
}

void TransferPropertiesDialog::setTransferPriority(int index) {
    if (m_transfer) {
        m_transfer->setPriority(static_cast<Transfers::Priority>(index));
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
    case Transfers::Canceled:
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
        m_connectionsSelector->setCurrentIndex(m_transfer->preferredConnections() - 1);
        return;
    case Transfer::CategoryRole:
        m_categorySelector->setCurrentIndex(m_categorySelector->findText(m_transfer->category()));
        return;
    case Transfer::PriorityRole:
        m_prioritySelector->setCurrentIndex(Transfers::Priority(m_transfer->priority()));
        return;
    default:
        return;
    }
}
