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

#include "packagetransferdelegate.h"
#include "../shared/transfer.h"
#include "../shared/definitions.h"
#include <QApplication>
#include <QStyleOptionProgressBar>
#include <QPainter>

PackageTransferDelegate::PackageTransferDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void PackageTransferDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
    QImage icon(index.data(Transfer::IconRole).toString());

    if (icon.isNull()) {
        icon = QImage(ICON_PATH + "qdl.png");
    }

    painter->drawImage(option.rect.left() + 8, option.rect.top() + 25, icon.scaled(36, 36, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    QRect textRect(option.rect.left() + 52, option.rect.top(), option.rect.width() - 218, option.rect.height());

    painter->setClipRect(textRect);
    painter->drawText(textRect, Qt::TextSingleLine | Qt::AlignVCenter, index.data(Transfer::NameRole).toString());
    painter->setClipping(false);

    int progress = index.data(Transfer::ProgressRole).toInt();

    QStyleOptionProgressBar progressBar;
    progressBar.rect = option.rect;
    progressBar.rect.setTop(option.rect.top() + 8);
    progressBar.rect.setHeight(70);
    progressBar.rect.setLeft(option.rect.right() - 158);
    progressBar.rect.setWidth(150);
    progressBar.minimum = 0;
    progressBar.maximum = 100;
    progressBar.progress = progress;
    progressBar.textVisible = true;
    progressBar.text = QString::number(progress) + "%";

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBar, painter);
}

QSize PackageTransferDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(index)

    return QSize(option.rect.width(), 86);
}
