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

#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QQueue>
#include <QStringList>

class Settings;
class Database;
class UrlChecker;
class UrlRetriever;
class PluginManager;
class NetworkAccessManager;
class ClipboardMonitor;
class TransferModel;
#ifdef WEB_INTERFACE
class WebInterface;
#endif

class Session : public QObject
{
    Q_OBJECT

public:
    explicit Session(QObject *parent = 0);
    ~Session();
    
private:
    NetworkAccessManager *m_nam;
    Settings *m_settings;
    Database *m_database;
    UrlChecker *m_urlChecker;
    UrlRetriever *m_urlRetriever;
    PluginManager *m_pluginManager;
    ClipboardMonitor *m_clipboardMonitor;
    TransferModel *m_model;
#ifdef WEB_INTERFACE
    WebInterface *m_webIf;
#endif
};

#endif // SESSION_H
