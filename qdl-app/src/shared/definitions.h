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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <QString>
#include <QList>
#include <QSize>

static const QString VERSION_NUMBER("1.2.1");
static const QString ICON_PATH("/opt/qdl/icons/");
#if (defined Q_WS_MAEMO_5) || (defined MAEMO4_OS)
static const QSize ICON_SIZE(36, 36);
#else
static const QSize ICON_SIZE(16, 16);
#endif
static const int MAX_CONCURRENT_TRANSFERS = 5;
static const int MAX_CONNECTIONS = 4;
static const QList<int> RATE_LIMITS = QList<int>() << 0 << 5000 << 10000 << 20000 << 50000 << 100000 << 250000 << 500000 << 750000;

#endif // DEFINITIONS_H
