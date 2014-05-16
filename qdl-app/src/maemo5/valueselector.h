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

#ifndef VALUESELECTOR_H
#define VALUESELECTOR_H

#include <QMaemo5ValueButton>

class QMaemo5ListPickSelector;
class SelectionModel;

class ValueSelector : public QMaemo5ValueButton
{
    Q_OBJECT

public:
    explicit ValueSelector(const QString &text, QWidget *parent = 0);
    ~ValueSelector();

    SelectionModel* model() const;
    virtual void setModel(SelectionModel *model);
    
    QVariant currentValue();
    virtual void setValue(const QVariant &value);

protected slots:
    virtual void onSelected();

signals:
    void valueChanged(const QVariant &value);

protected:
    SelectionModel *m_model;
    QMaemo5ListPickSelector *m_selector;
};

#endif // VALUESELECTOR_H
