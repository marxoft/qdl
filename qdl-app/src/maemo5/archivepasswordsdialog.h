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

#ifndef ARCHIVEPASSWORDSDIALOG_H
#define ARCHIVEPASSWORDSDIALOG_H

#include <QDialog>

class ArchivePasswordsModel;
class QListView;
class QLineEdit;
class QPushButton;
class QMenu;

class ArchivePasswordsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ArchivePasswordsDialog(QWidget *parent = 0);
    ~ArchivePasswordsDialog();

private slots:
    void showContextMenu(const QPoint &pos);
    void onPasswordEditTextChanged(const QString &text);
    void addArchivePassword();
    void removeArchivePassword();
    
private:
    ArchivePasswordsModel *m_model;
    QListView *m_view;
    QMenu *m_contextMenu;
    QAction *m_removePasswordAction;
    QLineEdit *m_passwordEdit;
    QPushButton *m_passwordButton;
};

#endif // ARCHIVEPASSWORDSDIALOG_H
