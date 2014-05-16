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

#include "captchaimageprovider.h"
#include <QDebug>

CaptchaImageProvider::CaptchaImageProvider() :
    QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap)
{
}

QPixmap CaptchaImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
    if (size) {
        *size = QSize(400, 200);
    }

    QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : 400,
                   requestedSize.height() > 0 ? requestedSize.height() : 200);

    if (!pixmap.loadFromData(QByteArray::fromBase64(id.toAscii()))) {
        qDebug() << "Cannot load pixmap from data";
    }

    return pixmap;
}
