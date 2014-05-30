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

#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

class CookieJar;

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    static NetworkAccessManager* instance();
    static NetworkAccessManager* create(QObject *parent = 0);

private:
    NetworkAccessManager(QObject *parent = 0);
    ~NetworkAccessManager();
    
private:
    static NetworkAccessManager *self;
    static CookieJar *m_cookieJar;
};

#endif // NETWORKACCESSMANAGER_H
