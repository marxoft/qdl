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

#include "utils.h"
#include <QFileInfo>
#include <QFile>
#ifndef QML_USER_INTERFACE
#include <QPainter>
#include <QRect>
#include <QImage>
#endif

Utils::Utils(QObject *parent) :
    QObject(parent)
{
}

Utils::~Utils() {}

QString Utils::durationFromSecs(int secs) {
    QTime time(0, 0);
    QString format = secs >= 3600 ? "hh:mm:ss" : "mm:ss";

    return time.addSecs(secs).toString(format);
}

QString Utils::durationFromMSecs(int msecs) {
    QTime time(0, 0);
    QString format = msecs >= 3600000 ? "hh:mm:ss" : "mm:ss";

    return time.addMSecs(msecs).toString(format);
}

QString Utils::fileSizeFromPath(const QString &filePath) {
    QFileInfo file(filePath);
    return Utils::fileSizeFromBytes(file.size());
}

QString Utils::fileSizeFromBytes(double bytes) {
    double kb = 1024;
    double mb = kb * 1024;
    double gb = mb * 1024;

    QString size;

    if (bytes > gb) {
        size = QString::number(bytes / gb, 'f', 2) + "GB";
    }
    else if (bytes > mb) {
        size = QString::number(bytes / mb, 'f', 2) + "MB";
    }
    else if (bytes > kb){
        size = QString::number(bytes / kb, 'f', 2) + "KB";
    }
    else {
        size = QString::number(bytes) + "B";
    }

    return size;
}

QString Utils::dateFromSecs(qint64 secs, bool showTime) {
    return Utils::dateFromMSecs(secs * 1000, showTime);
}

QString Utils::dateFromMSecs(qint64 msecs, bool showTime) {
    QString date;

    if (showTime) {
        date = Utils::dateTimeFromMSecsSinceEpoch(msecs).toString("dd/MM/yyyy | HH:mm");
    }
    else {
        date = Utils::dateTimeFromMSecsSinceEpoch(msecs).toString("dd/MM/yyyy");
    }

    return date;
}

QString Utils::base64Id(const QString &artist, const QString &title) {
    return QString(artist.toUtf8().toBase64() + "-" + title.toUtf8().toBase64());
}

QString Utils::localDateTimeFromString(const QString &dateTimeString, Qt::DateFormat format) {
    QDateTime dt = QDateTime::fromString(dateTimeString, format);

    if (!dt.isValid()) {
        dt = QDateTime::currentDateTime();
    }

    return dt.toLocalTime().toString("dd/MM/yyyy | HH:mm");
}

qint64 Utils::currentMSecsSinceEpoch() {
#if QT_VERSION >= 0x040700
    return QDateTime::currentMSecsSinceEpoch();
#else
    return qint64(QDateTime::currentDateTime().toTime_t()) * 1000;
#endif
}

qint64 Utils::dateTimeToMSecsSinceEpoch(const QDateTime &dateTime) {
#if QT_VERSION >= 0x040700
    return dateTime.toMSecsSinceEpoch();
#else
    return qint64(dateTime.toTime_t()) * 1000;
#endif
}

QDateTime Utils::dateTimeFromMSecsSinceEpoch(qint64 msecs) {
#if QT_VERSION >= 0x040700
    return QDateTime::fromMSecsSinceEpoch(msecs);
#else
    return QDateTime::fromTime_t(msecs / 1000);
#endif
}

QString Utils::httpErrorString(int errorCode) {
    switch (errorCode) {
    case 400:
        return tr("Bad request");
    case 401:
        return tr("Request is unauthorised");
    case 403:
        return tr("Request is forbidden");
    case 404:
        return tr("Requested resource is unavailable");
    case 406:
        return tr("Requested resource is not accessible");
    case 422:
        return tr("Request cannot be processed");
    case 429:
        return tr("Request limit has been reached. Please try again later");
    case 500:
        return tr("Internal server error. Please try again later");
    case 503:
        return tr("Service unavailable. Please try again later");
    case 504:
        return tr("Request timed out. Please try again later");
    default:
        return tr("Unknown error. Please try again later");
    }
}

void Utils::log(const QString &filePath, const QByteArray &message) {
    QFile lf(filePath);

    if (lf.open(QIODevice::Append)) {
        lf.write(QDateTime::currentDateTime().toString().toUtf8() + ": " + message + "\n");
    }

    lf.close();
}

#ifndef QML_USER_INTERFACE
void Utils::drawBorderImage(QPainter *painter, const QRect &rect, const QImage &image, int top, int right, int left, int bottom) {
    QRect imageRect = image.rect();

    // Top-left
    painter->drawImage(QRect(rect.left(), rect.top(), left, top),
                       image,
                       QRect(imageRect.left(), imageRect.top(), left, top));

    // Top-right
    painter->drawImage(QRect(rect.right() - right, rect.top(), right, top),
                       image,
                       QRect(imageRect.right() - right, imageRect.top(), right, top));

    // Bottom-left
    painter->drawImage(QRect(rect.left(), rect.bottom() - bottom, left, bottom),
                       image,
                       QRect(imageRect.left(), imageRect.bottom() - bottom, left, bottom));

    // Bottom-right
    painter->drawImage(QRect(rect.right() - right, rect.bottom() - bottom, bottom, right),
                       image,
                       QRect(imageRect.right() - right, imageRect.bottom() - bottom, right, bottom));

    // Top-middle
    painter->drawImage(QRect(rect.left() + left, rect.top(), rect.width() - (left + right), top),
                       image,
                       QRect(imageRect.left() + left, imageRect.top(), imageRect.width() - (left + right), top));

    // Bottom-middle
    painter->drawImage(QRect(rect.left() + left, rect.bottom() - bottom, rect.width() - (left + right), bottom),
                       image,
                       QRect(imageRect.left() + left, imageRect.bottom() - bottom, imageRect.width() - (left + right), bottom));

    // Left-middle
    painter->drawImage(QRect(rect.left(), rect.top() + top, left, rect.height() - (top + bottom)),
                       image,
                       QRect(imageRect.left(), imageRect.top() + top, left, imageRect.height() - (top + bottom)));

    // Right-middle
    painter->drawImage(QRect(rect.right() - right, rect.top() + top, right, rect.height() - (top + bottom)),
                       image,
                       QRect(imageRect.right() - right, imageRect.top() + top, right, imageRect.height() - (top + bottom)));

    // Center
    painter->drawImage(QRect(rect.left() + left, rect.top() + top, rect.width() - (left + right), rect.height() - (top + bottom)),
                       image,
                       QRect(imageRect.left() + left, imageRect.top() + top, imageRect.width() - (left + right), imageRect.height() - (top + bottom)));
}
#endif
