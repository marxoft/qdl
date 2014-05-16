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

#ifndef TRANSFERCATEGORYDIALOG_H
#define TRANSFERCATEGORYDIALOG_H

#include <QDialog>

class CategoriesModel;
class QListView;
class QModelIndex;

class TransferCategoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransferCategoryDialog(QWidget *parent = 0);
    ~TransferCategoryDialog();

    QString currentValue() const;
    void setValue(const QString &value);

private slots:
    void onItemClicked(const QModelIndex &index);

signals:
    void valueChanged(const QString &value);
    
private:
    CategoriesModel *m_model;
    QListView *m_listView;
};

#endif // TRANSFERCATEGORYDIALOG_H
