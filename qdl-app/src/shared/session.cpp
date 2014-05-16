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

#include "session.h"
#include "networkaccessmanager.h"
#include "settings.h"
#include "database.h"
#include "storage.h"
#include "urlchecker.h"
#include "urlretriever.h"
#include "pluginmanager.h"
#include "clipboardmonitor.h"
#include "transfermodel.h"
#ifdef WEB_INTERFACE
#include "../webif/webinterface.h"
#endif

Session::Session(QObject *parent) :
    QObject(parent),
    m_nam(NetworkAccessManager::instance()),
    m_settings(Settings::instance()),
    m_database(Database::instance()),
    m_storage(Storage::instance()),
    m_urlChecker(UrlChecker::instance()),
    m_urlRetriever(UrlRetriever::instance()),
    m_pluginManager(PluginManager::instance()),
    m_clipboardMonitor(ClipboardMonitor::instance()),
    m_model(TransferModel::instance())
  #ifdef WEB_INTERFACE
    ,m_webIf(WebInterface::instance())
  #endif
{
    m_nam->setParent(this);
    m_settings->setParent(this);
    m_database->setParent(this);
    m_storage->setParent(this);
    m_urlChecker->setParent(this);
    m_urlRetriever->setParent(this);
    m_pluginManager->setParent(this);
    m_clipboardMonitor->setParent(this);
    m_model->setParent(this);
#ifdef WEB_INTERFACE
    m_webIf->setParent(this);
#endif
}

Session::~Session() {}
