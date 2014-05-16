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

#ifndef TRANSFERPROPERTIESDIALOG_H
#define TRANSFERPROPERTIESDIALOG_H

#include "../shared/transfer.h"
#include <QDialog>

class QLabel;
class QProgressBar;
class QCheckBox;
class ValueSelector;

class TransferPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransferPropertiesDialog(Transfer *transfer, QWidget *parent = 0);
    ~TransferPropertiesDialog();

private slots:
    void setTransferConnections(const QVariant &value);
    void setTransferCategory(const QVariant &value);
    void setTransferPriority(const QVariant &value);
    void setTransferConvertToAudio(bool convert);
    void onTransferStatusChanged(Transfers::Status status);
    void onTransferDataChanged(int role);

private:
    Transfer *m_transfer;
    QLabel *m_progressLabel;
    QLabel *m_statusLabel;
    QCheckBox *m_convertToAudioCheckBox;
    ValueSelector *m_connectionsSelector;
    ValueSelector *m_categorySelector;
    ValueSelector *m_prioritySelector;
    QProgressBar *m_progressBar;
    QPushButton *m_startButton;
    QPushButton *m_pauseButton;
    QPushButton *m_removeButton;
};

#endif // TRANSFERPROPERTIESDIALOG_H
