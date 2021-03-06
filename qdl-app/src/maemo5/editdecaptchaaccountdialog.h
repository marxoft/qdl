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

#ifndef EDITDECAPTCHAACCOUNTDIALOG_H
#define EDITDECAPTCHAACCOUNTDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;
class QCheckBox;

class EditDecaptchaAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditDecaptchaAccountDialog(QWidget *parent = 0);
    ~EditDecaptchaAccountDialog();

    void setAccount(const QString &icon, const QString &serviceName, const QString &username, const QString &password);

private slots:
    void onAccountTextChanged();
    void onActiveChanged(bool active);
    void submitAccount();

signals:
    void addAccount(const QString &serviceName, const QString &username, const QString &password);

private:
    QLabel *m_iconLabel;
    QLabel *m_nameLabel;
    QLineEdit *m_userEdit;
    QLineEdit *m_passEdit;
    QCheckBox *m_activeCheckbox;
    QPushButton *m_doneButton;
};

#endif // EDITDECAPTCHAACCOUNTDIALOG_H
