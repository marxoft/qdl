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

#ifndef ENUMS_H
#define ENUMS_H

#include <QObject>
#include <QNetworkProxy>

class ScreenOrientation : public QObject
{
    Q_OBJECT

    Q_ENUMS(Orientation)

public:
    enum Orientation {
#ifdef SAILFISH_OS
        LockPortrait = 1,
        LockLandscape = 2,
        LockPortraitInverted = 4,
        LockLandcapeInverted = 8,
        Automatic = 15
#else
        Automatic = 0,
        LockPortrait,
        LockLandscape
#endif
    };

    inline static QString orientationString(Orientation orientation) {
        switch (orientation) {
            case Automatic:
            return tr("Automatic");
        case LockPortrait:
            return tr("Portrait");
        case LockLandscape:
            return tr("Landscape");
#ifdef SAILFISH_OS
        case LockPortraitInverted:
            return tr("Portrait (inverted)");
        case LockLandscapeInverted:
            return tr("Landscape (inverted)");
        default:
            return tr("Unknown");
#endif
        }
    }
};

class Transfers : public QObject
{
    Q_OBJECT

    Q_ENUMS(Status Priority Action)

public:
    enum Status {
        Paused = 0,
        LongWait,
        Cancelled,
        Failed,
        Completed,
        Queued,
        Connecting,
        ShortWait,
        Downloading,
        Converting,
        Extracting,
        Unknown
    };

    enum Priority {
        HighPriority = 0,
        NormalPriority,
        LowPriority
    };

    enum Action {
        Continue = 0,
        Pause,
        Quit
    };

    inline static QString statusString(Status status) {
        switch (status) {
        case Queued:
            return tr("Queued");
        case Paused:
            return tr("Paused");
        case Connecting:
            return tr("Connecting");
        case ShortWait:
            return tr("Waiting");
        case LongWait:
            return tr("Waiting");
        case Downloading:
            return tr("Downloading");
        case Cancelled:
            return tr("Cancelled");
        case Failed:
            return tr("Failed");
        case Completed:
            return tr("Completed");
        case Converting:
            return tr("Converting");
        case Extracting:
            return tr("Extracting");
        default:
            return tr("Unknown");
        }
    }

    inline static QString priorityString(Priority priority) {
        switch (priority) {
        case HighPriority:
            return tr("High");
        case NormalPriority:
            return tr("Normal");
        case LowPriority:
            return tr("Low");
        default:
            return QString();
        }
    }

    inline static QString actionString(Action action) {
        switch (action) {
        case Continue:
            return tr("Continue");
        case Pause:
            return tr("Pause");
        case Quit:
            return tr("Quit");
        default:
            return QString();
        }
    }
};

class NetworkProxyType : public QObject
{
    Q_OBJECT

    Q_ENUMS(ProxyType)

public:
    enum ProxyType {
        Socks5Proxy = QNetworkProxy::Socks5Proxy,
        HttpProxy = QNetworkProxy::HttpProxy,
        HttpCachingProxy = QNetworkProxy::HttpCachingProxy
    };

    inline static QString networkProxyTypeString(ProxyType type) {
        switch (type) {
        case Socks5Proxy:
            return QString("SOCKS5");
        case HttpProxy:
            return QString("HTTP");
        case HttpCachingProxy:
            return QString("HTTP %1").arg(tr("caching"));
        default:
            return QString("HTTP");
        }
    }
};

#endif // ENUMS_H
