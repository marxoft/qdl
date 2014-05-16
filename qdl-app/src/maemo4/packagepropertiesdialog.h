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

#ifndef PACKAGEPROPERTIESDIALOG_H
#define PACKAGEPROPERTIESDIALOG_H

#include "../shared/transfer.h"
#include <QDialog>

class PackageTransferModel;
class QListView;
class QComboBox;

class PackagePropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PackagePropertiesDialog(Transfer *package, QWidget *parent = 0);
    ~PackagePropertiesDialog();

private slots:
    void onTransferCountChanged(int count);
    void onPackageDataChanged(int role);
    void setPackageCategory(const QString &text);
    void setPackagePriority(int index);
    
private:
    Transfer *m_package;
    PackageTransferModel *m_model;
    QListView *m_view;
    QComboBox *m_categorySelector;
    QComboBox *m_prioritySelector;
    QPushButton *m_startButton;
    QPushButton *m_pauseButton;
    QPushButton *m_removeButton;
};

#endif // PACKAGEPROPERTIESDIALOG_H
