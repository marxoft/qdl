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

#ifndef MASKEDITEM_H
#define MASKEDITEM_H

#include "maskeffect.h"
#include <QDeclarativeItem>

class MaskEffect;
class QDeclarativeComponent;


class MaskedItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativeComponent *mask
               READ mask
               WRITE setMask
               NOTIFY maskChanged)

public:
    MaskedItem(QDeclarativeItem *parent = 0);
    virtual ~MaskedItem();

    QDeclarativeComponent *mask() const;
    void setMask(QDeclarativeComponent *component);

signals:
    void maskChanged();

private:
    MaskEffect *m_effect;
    QDeclarativeComponent *m_maskComponent;
};

#endif // MASKEDITEM_H
