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

#ifndef NETWORKPROXYDIALOG_H
#define NETWORKPROXYDIALOG_H

#include <QDialog>

class QCheckBox;
class QLineEdit;
class ValueSelector;

class NetworkProxyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkProxyDialog(QWidget *parent = 0);
    ~NetworkProxyDialog();

private:
    void loadSettings();

private slots:
    void saveSettings();
    void onProxyCheckboxToggled(bool checked);
    
private:
    QCheckBox *m_proxyCheckbox;
    QWidget *m_proxyWidget;
    ValueSelector *m_proxyTypeSelector;
    QLineEdit *m_hostEdit;
    QLineEdit *m_portEdit;
    QLineEdit *m_userEdit;
    QLineEdit *m_passEdit;
};

#endif // NETWORKPROXYDIALOG_H
