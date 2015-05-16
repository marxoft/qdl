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

#include "mainwindow.h"
#include "../shared/transfer.h"
#include "../shared/session.h"
#include "../shared/definitions.h"
#include "../dbus/dbusservice.h"
#include "../dbus/dbusserviceadaptor.h"
#include <QApplication>
#if QT_VERSION < 0x050000
#include <QSsl>
#include <QSslConfiguration>
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("QDL");
    app.setApplicationVersion(VERSION_NUMBER);
    app.setWindowIcon(QIcon::fromTheme("qdl"));
    app.setQuitOnLastWindowClosed(false);
    
#if QT_VERSION < 0x050000
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    QSslConfiguration::setDefaultConfiguration(config);
#endif

    QIcon::setThemeName("Lubuntu");

    qRegisterMetaType< QList<Transfer*> >("QList<Transfer*>");
    qRegisterMetaType< QList<QUrl> >("QList<QUrl>");

    Session session;

    DBusServiceAdaptor adaptor(DBusService::instance());

    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("com.marxoft.QDL");
    connection.registerObject("/", DBusService::instance());

    MainWindow window;
    window.show();

    return app.exec();
}
