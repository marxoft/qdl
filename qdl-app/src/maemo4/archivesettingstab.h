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

#ifndef ARCHIVESETTINGSTAB_H
#define ARCHIVESETTINGSTAB_H

#include <QWidget>

class ArchivePasswordsModel;
class QLineEdit;
class QCheckBox;
class QListView;
class QPushButton;
class QMenu;

class ArchiveSettingsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ArchiveSettingsTab(QWidget *parent = 0);
    ~ArchiveSettingsTab();

public slots:
    void saveSettings();

private:
    void loadSettings();

private slots:
    void showContextMenu(const QPoint &pos);
    void onPasswordEditTextChanged(const QString &text);
    void addArchivePassword();
    void removeArchivePassword();
    
private:
    QCheckBox *m_extractCheckbox;
    QCheckBox *m_subfoldersCheckbox;
    QCheckBox *m_deleteCheckbox;
    ArchivePasswordsModel *m_model;
    QListView *m_view;
    QMenu *m_contextMenu;
    QAction *m_removePasswordAction;
    QLineEdit *m_passwordEdit;
    QPushButton *m_passwordButton;
};

#endif // ARCHIVESETTINGSTAB_H
