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
#ifdef TABLE_TRANSFER_VIEW
    if (index.column() == 4) {
        QStyleOptionProgressBar progressBar;
        progressBar.rect = option.rect;
        progressBar.minimum = 0;
        progressBar.maximum = 100;
        progressBar.progress = index.data(Transfer::ProgressRole).toInt();
        progressBar.textVisible = true;
        progressBar.text = QString::number(progressBar.progress) + "%";
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBar, painter);
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
#else
    QStyledItemDelegate::paint(painter, option, index);
    QImage icon(index.data(Transfer::IconRole).toString());

    if (icon.isNull()) {
        icon = QImage(ICON_PATH + "qdl.png");
    }

    painter->drawImage(option.rect.left() + 8, option.rect.top() + 8,
                       icon.scaled(36, 36, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    painter->drawText(option.rect.left() + 52, option.rect.top() + 8, option.rect.width() - 218,
                      98, Qt::TextWrapAnywhere, index.data(Transfer::NameRole).toString());

    QFont font;
    font.setPixelSize(18);

    painter->save();
    painter->setFont(font);

    qint64 size = index.data(Transfer::SizeRole).toLongLong();

    switch (index.data(Transfer::StatusRole).toInt()) {
    case Transfers::Failed:
        painter->setPen(Qt::red);
        break;
    default:
        if (size > 0) {
            painter->drawText(option.rect.left() + 8, option.rect.bottom() - 36, option.rect.width() - 16,
                              28, Qt::AlignBottom | Qt::AlignRight,
                              QString("%1 of %2").arg(Utils::fileSizeFromBytes(index.data(Transfer::PositionRole).toLongLong()))
                              .arg(Utils::fileSizeFromBytes(size)));
        }

        break;
    }

    painter->drawText(option.rect.left() + 8, option.rect.bottom() - 36, option.rect.width() - 16, 28, Qt::AlignBottom,
                      index.data(Transfer::StatusStringRole).toString());

    painter->restore();

    QStyleOptionProgressBar progressBar;
    progressBar.rect = option.rect;
    progressBar.rect.setTopLeft(QPoint(progressBar.rect.right() -158, progressBar.rect.top() + 36));
    progressBar.rect.setWidth(150);
    progressBar.rect.setHeight(70);
    progressBar.minimum = 0;
    progressBar.maximum = 100;
    progressBar.progress = index.data(Transfer::ProgressRole).toInt();
    progressBar.textVisible = true;
    progressBar.text = QString::number(progressBar.progress) + "%";

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBar, painter);
#endif
}

#ifndef TABLE_TRANSFER_VIEW
QSize TransferItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(index)

    return QSize(option.rect.width(), 142);
}
#endif
