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

#include "cookiejar.h"
#include <QRegExp>
#include <QUrl>

CookieJar::CookieJar(QObject *parent) :
    QNetworkCookieJar(parent)
{
}

QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const {
    QList<QNetworkCookie> cookies = QNetworkCookieJar::cookiesForUrl(url);

    if (url.toString().contains(QRegExp("(dailymotion.com/video|dailymotion.com/cdn|dai.ly)"))) {
        cookies << QNetworkCookie("family_filter", "false");
    }

    return cookies;
}
