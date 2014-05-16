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

#ifndef PROXYSETTINGSTAB_H
#define PROXYSETTINGSTAB_H

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLineEdit;

class ProxySettingsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ProxySettingsTab(QWidget *parent = 0);
    ~ProxySettingsTab();

public slots:
    void saveSettings();

private:
    void loadSettings();

private slots:
    void onProxyCheckboxToggled(bool checked);
    
private:
    QCheckBox *m_proxyCheckbox;
    QComboBox *m_proxyTypeCombobox;
    QLineEdit *m_proxyHostEdit;
    QLineEdit *m_proxyPortEdit;
    QLineEdit *m_proxyUserEdit;
    QLineEdit *m_proxyPassEdit;
};

#endif // PROXYSETTINGSTAB_H
