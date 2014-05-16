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
class QComboBox;
class QProgressBar;
class QCheckBox;

class TransferPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransferPropertiesDialog(Transfer *transfer, QWidget *parent = 0);
    ~TransferPropertiesDialog();
    
private slots:
    void onTransferStatusChanged(Transfers::Status status);
    void onTransferDataChanged(int role);
    void setTransferConnections(int index);
    void setTransferCategory(const QString &text);
    void setTransferPriority(int index);
    void setTransferConvertToAudio(bool convert);

private:
    Transfer *m_transfer;
    QCheckBox *m_convertToAudioCheckBox;
    QComboBox *m_connectionsSelector;
    QComboBox *m_categorySelector;
    QComboBox *m_prioritySelector;
    QLabel *m_progressLabel;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    QPushButton *m_startButton;
    QPushButton *m_pauseButton;
    QPushButton *m_removeButton;
};

#endif // TRANSFERPROPERTIESDIALOG_H
