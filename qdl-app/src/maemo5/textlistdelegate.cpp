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

#include "textlistdelegate.h"
#include <QStaticText>

TextListDelegate::TextListDelegate(int role, Qt::Alignment alignment, QObject *parent) :
    QStyledItemDelegate(parent),
    m_role(role),
    m_alignment(alignment)
{
}

void TextListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if ((option.state) & (QStyle::State_Selected)) {
        painter->drawImage(option.rect, QImage("/etc/hildon/theme/images/TouchListBackgroundPressed.png"));
    }
    else {
        painter->drawImage(option.rect, QImage("/etc/hildon/theme/images/TouchListBackgroundNormal.png"));
    }

    QRect rect = option.rect;
    rect.setLeft(rect.left() + 10);
    rect.setRight(rect.right() - 10);

    QStaticText text(index.data(m_role).toString());
    text.setTextOption(QTextOption(m_alignment));
    text.setTextWidth(rect.width());

    painter->drawStaticText(rect.left(), rect.center().y() - text.size().height() / 2, text);
}

QSize TextListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(index);

    return QSize(option.rect.width(), 70);
}
