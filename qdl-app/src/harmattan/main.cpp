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

#include "../shared/session.h"
#include "../shared/transfermodel.h"
#include "../shared/transferfiltermodel.h"
#include "../shared/pluginmanager.h"
#include "../shared/settings.h"
#include "../shared/urlchecker.h"
#include "../shared/urlretriever.h"
#include "../shared/clipboardmonitor.h"
#include "../shared/database.h"
#include "../shared/selectionmodels.h"
#include "../shared/categoriesmodel.h"
#include "../shared/serviceaccountsmodel.h"
#include "../shared/decaptchaaccountsmodel.h"
#include "../shared/pluginsettingsmodel.h"
#include "../shared/packagetransfermodel.h"
#include "../shared/archivepasswordsmodel.h"
#include "../shared/definitions.h"
#include "../shared/utils.h"
#include "../dbus/dbusservice.h"
#include "../dbus/dbusserviceadaptor.h"
#include "../webif/webinterfacethememodel.h"
#include "folderlistmodel.h"
#include "maskeditem.h"
#include <QApplication>
#include <MDeclarativeCache>
#include <QtDeclarative>
#include <QGLWidget>

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(MDeclarativeCache::qApplication(argc, argv));
    QScopedPointer<QDeclarativeView> view(MDeclarativeCache::qDeclarativeView());

    Session session;
    Utils utils;
    TransferFilterModel filterModel;
    DBusServiceAdaptor adaptor(DBusService::instance());

    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("com.marxoft.QDL");
    connection.registerObject("/", DBusService::instance());

    qRegisterMetaType< QList<Transfer*> >("QList<Transfer*>");
    qRegisterMetaType< QList<QUrl> >("QList<QUrl>");
    qRegisterMetaType<Transfers::Status>("Transfers::Status");
    qRegisterMetaType<Transfers::Action>("Transfers::Action");

    qmlRegisterType<QDeclarativeFolderListModel>("com.marxoft.models",1,0,"FolderListModel");
    qmlRegisterType<ScreenOrientationModel>("com.marxoft.models",1,0,"ScreenOrientationModel");
    qmlRegisterType<StatusFilterModel>("com.marxoft.models",1,0,"StatusFilterModel");
    qmlRegisterType<TransferActionModel>("com.marxoft.models",1,0,"TransferActionModel");
    qmlRegisterType<ConcurrentTransfersModel>("com.marxoft.models",1,0,"ConcurrentTransfersModel");
    qmlRegisterType<ConnectionsModel>("com.marxoft.models",1,0,"ConnectionsModel");
    qmlRegisterType<TransferPriorityModel>("com.marxoft.models",1,0,"TransferPriorityModel");
    qmlRegisterType<CategoriesModel>("com.marxoft.models",1,0,"CategoriesModel");
    qmlRegisterType<ServiceAccountsModel>("com.marxoft.models",1,0,"ServiceAccountsModel");
    qmlRegisterType<DecaptchaAccountsModel>("com.marxoft.models",1,0,"DecaptchaAccountsModel");
    qmlRegisterType<PluginSettingsModel>("com.marxoft.models",1,0,"PluginSettingsModel");
    qmlRegisterType<NetworkProxyTypeModel>("com.marxoft.models",1,0,"NetworkProxyTypeModel");
    qmlRegisterType<PackageTransferModel>("com.marxoft.models",1,0,"PackageTransferModel");
    qmlRegisterType<ArchivePasswordsModel>("com.marxoft.models",1,0,"ArchivePasswordsModel");
    qmlRegisterType<WebInterfaceThemeModel>("com.marxoft.models",1,0,"WebInterfaceThemeModel");
    qmlRegisterType<SelectionModel>("com.marxoft.models",1,0,"SelectionModel");
    qmlRegisterType<MaskedItem>("com.marxoft.items",1,0,"MaskedItem");
    qmlRegisterType<Transfer>("com.marxoft.items",1,0,"Transfer");

    qmlRegisterUncreatableType<ScreenOrientation>("com.marxoft.enums",1,0,"ScreenOrientation", "");
    qmlRegisterUncreatableType<Transfers>("com.marxoft.enums",1,0,"Transfers", "");
    qmlRegisterUncreatableType<NetworkProxyType>("com.marxoft.enums",1,0,"NetworkProxyType", "");

    QDeclarativeContext *context = view->rootContext();
    context->setContextProperty("TransferModel", TransferModel::instance());
    context->setContextProperty("TransferFilterModel", &filterModel);
    context->setContextProperty("UrlChecker", UrlChecker::instance());
    context->setContextProperty("UrlCheckModel", UrlChecker::instance()->model());
    context->setContextProperty("UrlRetriever", UrlRetriever::instance());
    context->setContextProperty("PluginManager", PluginManager::instance());
    context->setContextProperty("ClipboardMonitor", ClipboardMonitor::instance());
    context->setContextProperty("Database", Database::instance());
    context->setContextProperty("Settings", Settings::instance());
    context->setContextProperty("Utils", &utils);
    context->setContextProperty("versionNumber", VERSION_NUMBER);
    context->setContextProperty("iconPath", ICON_PATH);

    view->setViewport(new QGLWidget());
    view->setSource(QUrl("qrc:/main.qml"));
    view->showFullScreen();

    return app->exec();
}
