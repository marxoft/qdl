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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class ValueSelector;
class QMaemo5ValueButton;
class QCheckBox;
class QSpinBox;
class QModelIndex;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

private:
    void loadSettings();

private slots:
    void saveSettings();
    void showFileDialog();
    void showArchivePasswordsDialog();
    void showCategoriesDialog();
    void showServiceAccountsDialog();
    void showDecaptchaAccountsDialog();
    void showPluginSettingsDialog(const QModelIndex &index);
    void showNetworkProxyDialog();

private:
    QMaemo5ValueButton *m_pathSelector;
    QCheckBox *m_statusCheckbox;
    QCheckBox *m_clipboardCheckbox;
    QCheckBox *m_extractArchivesCheckbox;
    QCheckBox *m_archiveSubfoldersCheckbox;
    QCheckBox *m_deleteArchivesCheckbox;
    QCheckBox *m_webIfCheckbox;
    QSpinBox *m_webIfPortSelector;
    ValueSelector *m_webIfThemeSelector;
};

#endif // SETTINGSDIALOG_H
