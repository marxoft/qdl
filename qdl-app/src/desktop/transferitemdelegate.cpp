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

#include "transferitemdelegate.h"
#include "../shared/transfer.h"
#include "../shared/definitions.h"
#include "../shared/utils.h"
#include <QApplication>
#include <QStyleOptionProgressBar>
#include <QPainter>

TransferItemDelegate::TransferItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void TransferItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
    QImage icon(index.data(Transfer::IconRole).toString());

    if (icon.isNull()) {
        icon = QImage(ICON_PATH + "qdl.png");
    }

    painter->drawImage(option.rect.left() + 5, option.rect.top() + 5, icon.scaled(20, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    painter->save();
    QFont bold;
    bold.setBold(true);
    painter->setFont(bold);

    painter->drawText(option.rect.left() + 30, option.rect.top() + 5, option.rect.width() - 35, 20, Qt::TextSingleLine, index.data(Transfer::NameRole).toString());

    painter->restore();

    painter->drawText(option.rect.left() + 5, option.rect.top() + 30, option.rect.width() - 10, 20, Qt::TextSingleLine,
                      QString("%1: %2").arg(tr("Priority")).arg(index.data(Transfer::PriorityStringRole).toString()));

    qint64 size = index.data(Transfer::SizeRole).toLongLong();

    if (size > 0) {
        painter->drawText(option.rect.left() + 5, option.rect.top() + 30, option.rect.width() - 10, 20, Qt::TextSingleLine | Qt::AlignRight,
                          QString("%1 of %2 (%3%)").arg(Utils::fileSizeFromBytes(index.data(Transfer::PositionRole).toLongLong())).arg(Utils::fileSizeFromBytes(size)).arg(index.data(Transfer::ProgressRole).toInt()));
    }
    else {
        painter->drawText(option.rect.left() + 5, option.rect.top() + 30, option.rect.width() - 10, 20, Qt::TextSingleLine | Qt::AlignRight,
                          QString("%1 of %2").arg(Utils::fileSizeFromBytes(index.data(Transfer::PositionRole).toLongLong())).arg(tr("Unknown")));
    }

    painter->save();

    switch (index.data(Transfer::StatusRole).toInt()) {
    case Transfers::Failed:
        painter->setPen(Qt::red);
        break;
    default:
        break;
    }

    painter->drawText(option.rect.left() + 5, option.rect.bottom() - 25, option.rect.width() - 10, 20, Qt::TextSingleLine | Qt::AlignBottom,
                      index.data(Transfer::StatusStringRole).toString());

    painter->restore();

    QStyleOptionProgressBar progressBar;
    progressBar.rect = option.rect;
    progressBar.rect.setTopLeft(QPoint(progressBar.rect.left() + 5, progressBar.rect.top() + 55));
    progressBar.rect.setWidth(option.rect.width() - 10);
    progressBar.rect.setHeight(15);
    progressBar.minimum = 0;
    progressBar.maximum = 100;
    progressBar.progress = index.data(Transfer::ProgressRole).toInt();
    progressBar.textVisible = false;

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBar, painter);
}

QSize TransferItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(index)

    return QSize(option.rect.width(), 100);
}
