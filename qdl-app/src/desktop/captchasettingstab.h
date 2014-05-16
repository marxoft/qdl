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

#ifndef CAPTCHASETTINGSTAB_H
#define CAPTCHASETTINGSTAB_H

#include <QWidget>

class DecaptchaAccountsModel;
class QTreeView;
class QPushButton;
class QLineEdit;
class QCheckBox;
class QMenu;

class CaptchaSettingsTab : public QWidget
{
    Q_OBJECT

public:
    explicit CaptchaSettingsTab(QWidget *parent = 0);
    ~CaptchaSettingsTab();

private slots:
    void showContextMenu(const QPoint &pos);
    void addAccount();
    void removeAccount();
    void editAccount();
    void onAccountEditChanged();
    void onCheckboxClicked(bool checked);

private:
    DecaptchaAccountsModel *m_model;
    QTreeView *m_view;
    QMenu *m_contextMenu;
    QAction *m_editAction;
    QAction *m_removeAction;
    QLineEdit *m_nameEdit;
    QLineEdit *m_passEdit;
    QPushButton *m_doneButton;
    QCheckBox *m_checkbox;
};

#endif // CAPTCHASETTINGSTAB_H
