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

#ifndef NEWCATEGORYDIALOG_H
#define NEWCATEGORYDIALOG_H

#include <QDialog>

class QLineEdit;

class NewCategoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewCategoryDialog(QWidget *parent = 0);
    ~NewCategoryDialog();

public slots:
    void setName(const QString &name);
    void setPath(const QString &path);

private slots:
    void onNameTextChanged(const QString &text);
    void showFileDialog();
    void addCategory();

signals:
    void addCategory(const QString &name, const QString &path);
    
private:
    QLineEdit *m_nameEdit;
    QLineEdit *m_pathEdit;
    QPushButton *m_browseButton;
    QPushButton *m_doneButton;
};

#endif // NEWCATEGORYDIALOG_H
