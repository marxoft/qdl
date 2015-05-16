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

#include "webinterface.h"
#include "../qhttpserver/qhttpserver.h"
#include "../qhttpserver/qhttprequest.h"
#include "../qhttpserver/qhttpresponse.h"
#include "../shared/definitions.h"
#include "../shared/transfermodel.h"
#include "../shared/database.h"
#include "../shared/settings.h"
#include "../shared/urlchecker.h"
#include "../shared/urlretriever.h"
#include "../shared/pluginmanager.h"
#include "../interfaces/serviceplugin.h"
#include "../interfaces/decaptchaplugin.h"
#include "../json/json.h"
#include <QFile>
#include <QStringList>
#include <QCoreApplication>
#include <QDir>
#include <QTimer>
#include <QMetaEnum>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

using namespace QtJson;

WebInterface* WebInterface::self = 0;

WebInterface::WebInterface() :
    QObject(),
    m_server(new QHttpServer(this)),
    m_port(Settings::instance()->webInterfacePort()),
    m_path(QCoreApplication::applicationDirPath().section('/', 0, -2) + "/webif/")
{
    if (!self) {
        self = this;
    }

    this->connect(m_server, SIGNAL(newRequest(QHttpRequest*,QHttpResponse*)), this, SLOT(onNewRequest(QHttpRequest*,QHttpResponse*)));
    this->connect(Settings::instance(), SIGNAL(enableWebInterfaceChanged(bool)), this, SLOT(setEnabled(bool)));

    this->setEnabled(Settings::instance()->enableWebInterface());
}

WebInterface::~WebInterface() {}

WebInterface* WebInterface::instance() {
    return !self ? new WebInterface : self;
}

bool WebInterface::start() {
    this->loadHosts();
    return m_server->listen(this->port());
}

void WebInterface::stop() {
    m_server->close();
}

bool WebInterface::setEnabled(bool enable) {
    if (enable) {
        return this->start();
    }

    this->stop();
    return true;
}

quint16 WebInterface::port() const {
    return m_port;
}

void WebInterface::setPort(quint16 port) {
    m_port = port;
}

void WebInterface::loadHosts() {
    m_hosts.clear();
    QFile file(m_path + ".hosts");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QStringList hosts = QString(file.readAll()).split('\n', QString::SkipEmptyParts);
        file.close();

        foreach (QString host, hosts) {
            if (!host.startsWith('#')) {
                QHostAddress address(host);

                if (!address.isNull()) {
                    m_hosts << address;
                }
            }
        }
    }
    else {
        m_hosts << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::LocalHostIPv6);
    }
}

bool WebInterface::isAllowed(const QHostAddress &address) {
    return m_hosts.contains(address);
}

static QByteArray resource(const QString &fileName) {
    QByteArray data;
    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly)) {
        data = file.readAll();
        file.close();
    }

    return data;
}

static QByteArray status() {
    int i = Transfers::staticMetaObject.indexOfEnumerator("Action");
    QByteArray nextAction = Transfers::staticMetaObject.enumerator(i).key(Transfers::Action(TransferModel::instance()->nextAction()));

    return "{ \"totalTransfers\": " + QByteArray::number(TransferModel::instance()->rowCount()) \
            + ", \"activeTransfers\": " + QByteArray::number(TransferModel::instance()->activeTransfers()) \
            + ", \"maximumConcurrentTransfers\": " + QByteArray::number(Settings::instance()->maximumConcurrentTransfers()) \
            + ", \"maximumConnectionsPerTransfer\": " + QByteArray::number(Settings::instance()->maximumConnectionsPerTransfer()) \
            + ", \"downloadSpeed\": " + QByteArray::number(TransferModel::instance()->totalDownloadSpeed()) \
            + ", \"downloadRateLimit\": " + QByteArray::number(Settings::instance()->downloadRateLimit()) \
            + ", \"nextAction\": \"" + nextAction + "\"" \
            + ", \"versionNumber\": \"" + VERSION_NUMBER.toUtf8() + "\" }";
}

static QByteArray transfers(const QString &filter, const QString &query, int start, int count) {
    Transfers::Status statusFilter = Transfers::Unknown;

    if (!filter.isEmpty()) {
        int i = Transfers::staticMetaObject.indexOfEnumerator("Status");
        statusFilter = static_cast<Transfers::Status>(Transfers::staticMetaObject.enumerator(i).keyToValue(filter.toUtf8()));
    }

    return "{ \"next\": \"transfers?filter=" + filter.toUtf8() + "&query=" + query.toUtf8() + "&start=" + QByteArray::number(qMin(start + count, TransferModel::instance()->rowCount() - count)) + "&limit=" + QByteArray::number(count) + "\"" \
            + ", \"previous\": \"transfers?filter=" + filter.toUtf8() + "&query=" + query.toUtf8() + "&start=" +  QByteArray::number(qMax(0, start - count)) + "&limit=" + QByteArray::number(count) + "\"" \
            + ", \"transfers\": " + Json::serialize(TransferModel::instance()->allItemData(statusFilter, query, start, count)) + " }";
}

static QByteArray categoryNames() {
    return Json::serialize(Database::instance()->getCategoryNames());
}

static QByteArray categories() {
    QByteArray result = "[ ";

    QList< QPair<QString, QString> > list = Database::instance()->getCategories();

    while (!list.isEmpty()) {
        QPair<QString, QString> category = list.takeFirst();
        result += "{ \"name\": \"" + category.first + "\", \"path\": \"" + category.second + "\" }";

        if (!list.isEmpty()) {
            result += ", ";
        }
    }

    result += " ]";

    return result;
}

static QByteArray serviceNames() {
    return Json::serialize(PluginManager::instance()->servicePluginNames());
}

static QByteArray serviceAccounts() {
    QByteArray result = "[ ";

    for (int i = 0; i < PluginManager::instance()->servicePlugins().size(); i++) {
        ServicePlugin *plugin = PluginManager::instance()->servicePlugins().at(i);

        if (plugin->loginSupported()) {
            QPair<QString, QString> account = Database::instance()->getAccount(plugin->serviceName());
	    result += "{ \"serviceName\": \"" + plugin->serviceName() + "\", \"serviceIcon\": \"" + ICON_PATH + plugin->iconName() + "\", \"username\": \"" + account.first + "\", \"password\": \"" + account.second + "\" }";
            
	    if (i < (PluginManager::instance()->servicePlugins().size() - 1)) {
		result += ", ";
	    }
        }
    }

    result += " ]";

    return result;
}

static QByteArray decaptchaAccounts() {
    QByteArray result = "[ ";

    for (int i = 0; i < PluginManager::instance()->decaptchaPlugins().size(); i++) {
        DecaptchaPlugin *plugin = PluginManager::instance()->decaptchaPlugins().at(i);

        QPair<QString, QString> account = Database::instance()->getAccount(plugin->serviceName());
	result += "{ \"serviceName\": \"" + plugin->serviceName() + "\", \"serviceIcon\": \"" + ICON_PATH + plugin->iconName() + "\", \"username\": \"" + account.first + "\", \"password\": \"" + account.second + "\" }";
        
	if (i < (PluginManager::instance()->decaptchaPlugins().size() - 1)) {
	    result += ", ";
	}
    }

    result += " ]";

    return result;
}

static QByteArray urlCheckProgress() {
    return "{ \"progress\": " + QByteArray::number(UrlChecker::instance()->progress()) \
            + ", \"urls\": " + Json::serialize(UrlChecker::instance()->model()->allItemData()) + " }";
}

static QByteArray urlRetrievalProgress() {
    return "{ \"progress\": " + QByteArray::number(UrlRetriever::instance()->progress()) \
            + ", \"urls\": " + Json::serialize(UrlRetriever::instance()->resultsString().split("\n", QString::SkipEmptyParts)) + " }";
}

static QByteArray success() {
    return "{ \"result\": 200 }";
}

static QByteArray error(const QString &message) {
    return QString("{ \"error\": \"%1\" }").arg(message).toUtf8();
}

void WebInterface::onNewRequest(QHttpRequest *request, QHttpResponse *response) {
#if QT_VERSION >= 0x050000
    QUrl url = request->url();
    QUrlQuery query(url);
    QString path = request->path();
    QByteArray data;

    if (!this->isAllowed(QHostAddress(request->remoteAddress()))) {
        response->writeHead(403);
        data = error(tr("Forbidden"));
    }
    else if (path == "/status") {
        response->writeHead(200);
        data = status();
    }
    else if (path == "/transfers") {
        response->writeHead(200);
        data = transfers(query.queryItemValue("filter"), query.queryItemValue("query"), query.queryItemValue("start").toInt(), query.queryItemValue("limit").toInt());
    }
    else if (path == "/categoryNames") {
        response->writeHead(200);
        data = categoryNames();
    }
    else if (path == "/categories") {
        response->writeHead(200);
        data = categories();
    }
    else if (path == "/serviceNames") {
        response->writeHead(200);
        data = serviceNames();
    }
    else if (path == "/serviceAccounts") {
        response->writeHead(200);
        data = serviceAccounts();
    }
    else if (path == "/decaptchaAccounts") {
        response->writeHead(200);
        data = decaptchaAccounts();
    }
    else if (path.mid(1).startsWith("urls")) {
        QString method = path.section('/', -1);

        if (method == "addUrls") {
            QStringList urls = query.queryItemValue("urls").split(',', QString::SkipEmptyParts);

            if (!urls.isEmpty()) {
                if (query.hasQueryItem("category")) {
                    Settings::instance()->setDefaultCategory(query.queryItemValue("category"));
                }

                UrlChecker::instance()->addUrlsToQueue(urls, query.queryItemValue("service"));
                response->writeHead(200);
                data = urlCheckProgress();
            }
            else {
                response->writeHead(406);
                data = error(tr("No URLs specified"));
            }
        }
        else if (method == "retrieveUrls") {
            QStringList urls = query.queryItemValue("urls").split(',', QString::SkipEmptyParts);

            if (!urls.isEmpty()) {
                UrlRetriever::instance()->addUrlsToQueue(urls);
                response->writeHead(200);
                data = urlRetrievalProgress();
            }
            else {
                response->writeHead(406);
                data = error(tr("No URLs specified"));
            }
        }
        else if (method == "cancelUrlChecks") {
            UrlChecker::instance()->cancel();
            response->writeHead(200);
            data = success();
        }
        else if (method == "clearUrlChecks") {
            UrlChecker::instance()->model()->clear();
            response->writeHead(200);
            data = success();
        }
        else if (method == "cancelUrlRetrieval") {
            UrlRetriever::instance()->cancel();
            response->writeHead(200);
            data = success();
        }
        else if (method == "clearRetrievedUrls") {
            UrlRetriever::instance()->clearResults();
            response->writeHead(200);
            data = success();
        }
        else if (method == "urlCheckProgress") {
            response->writeHead(200);
            data = urlCheckProgress();
        }
        else if (method == "urlRetrievalProgress") {
            response->writeHead(200);
            data = urlRetrievalProgress();
        }
    }
    else if (path.mid(1).startsWith("transfers")) {
        QString method = path.section('/', -1);

        if (method == "start") {
            if (query.hasQueryItem("id")) {
                if (TransferModel::instance()->start(query.queryItemValue("id"))) {
                    response->writeHead(200);
                    data = Json::serialize(TransferModel::instance()->itemData(query.queryItemValue("id")));
                }
                else {
                    response->writeHead(404);
                    data = error(tr("Transfer not found"));
                }
            }
            else {
                TransferModel::instance()->start();
                response->writeHead(200);
                data = success();
            }
        }
        else if (method == "pause") {
            if (query.hasQueryItem("id")) {
                if (TransferModel::instance()->pause(query.queryItemValue("id"))) {
                    response->writeHead(200);
                    data = Json::serialize(TransferModel::instance()->itemData(query.queryItemValue("id")));
                }
                else {
                    response->writeHead(404);
                    data = error(tr("Transfer not found"));
                }
            }
            else {
                TransferModel::instance()->pause();
                response->writeHead(200);
                data = success();
            }
        }
        else if ((method == "cancel") || (method == "remove")) {
            if (TransferModel::instance()->cancel(query.queryItemValue("id"))) {
                response->writeHead(200);
                data = success();
            }
            else {
                response->writeHead(404);
                data = error(tr("Transfer not found"));
            }
        }
        else if (method == "getTransferProperty") {
            QVariant variant = TransferModel::instance()->data(query.queryItemValue("id"), query.queryItemValue("property").toUtf8());

            if (!variant.isNull()) {
                response->writeHead(200);

                switch (variant.type()) {
                case QVariant::String:
                    data = variant.toString().toUtf8();
                    break;
                default:
                    data = Json::serialize(variant);
                }
            }
            else {
                response->writeHead(404);
                data = error(tr("Transfer not found"));
            }
        }
        else if (method == "setTransferProperty") {
            if (TransferModel::instance()->setData(query.queryItemValue("id"), query.queryItemValue("value"), query.queryItemValue("property").toUtf8())) {
                response->writeHead(200);
                data = Json::serialize(TransferModel::instance()->itemData(query.queryItemValue("id")));
            }
            else {
                response->writeHead(406);
                data = error(tr("Transfer property could not be set"));
            }
        }
        else if (method == "getProperty") {
            QVariant variant = TransferModel::instance()->property(query.queryItemValue("property").toUtf8());

            if (!variant.isNull()) {
                response->writeHead(200);

                switch (variant.type()) {
                case QVariant::String:
                    data = variant.toString().toUtf8();
                    break;
                default:
                    data = Json::serialize(variant);
                }
            }
            else {
                response->writeHead(404);
                data = error(tr("Property not found"));
            }
        }
        else if (method == "setProperty") {
            if (TransferModel::instance()->setProperty(query.queryItemValue("property").toUtf8(), query.queryItemValue("value"))) {
                response->writeHead(200);
                data = success();
            }
            else {
                response->writeHead(406);
                data = error(tr("Property could not be set"));
            }
        }
        else if (method == "quit") {
            response->writeHead(200);
            data = success();
            TransferModel::instance()->pause();
            QTimer::singleShot(1000, QCoreApplication::instance(), SLOT(quit()));
        }
        else {
            response->writeHead(404);
            data = error(tr("Method '%1' not found").arg(method));
        }
    }
    else if (path.mid(1).startsWith("preferences")) {
        QString method = path.section('/', -1);

        if (method == "getProperties") {
            QStringList properties = query.queryItemValue("properties").split(",", QString::SkipEmptyParts);

            if (!properties.isEmpty()) {
                response->writeHead(200);

                QVariantMap map;

                foreach (QString property, properties) {
                    map[property] = Settings::instance()->property(property.toUtf8());
                }

                data = Json::serialize(map);
            }
            else {
                response->writeHead(404);
                data = error(tr("No properties specified"));
            }
        }
        else if (method == "setProperties") {
            QList< QPair<QString, QString> > queryItems = query.queryItems();

            int head = queryItems.isEmpty() ? 406 : 200;

            while (!queryItems.isEmpty()) {
                QPair<QString, QString> queryItem = queryItems.takeFirst();

                if (!Settings::instance()->setProperty(queryItem.first.toUtf8(), queryItem.second)) {
                    head = 406;
                }
            }

            data = (head == 200 ? success() : error(tr("Some properties could not be set")));
        }
        else {
            response->writeHead(404);
            data = error(tr("Method '%1' not found").arg(method));
        }
    }
    else if (path.mid(1).startsWith("categories")) {
        QString method = path.section('/', -1);

        if (method == "addCategory") {
            if (Database::instance()->addCategory(query.queryItemValue("name"), query.queryItemValue("path"))) {
                response->writeHead(200);
                data = "{ \"name\": \"" + query.queryItemValue("name").toUtf8() + "\", \"path\": \"" + query.queryItemValue("path").toUtf8() + "\" }";
            }
            else {
                response->writeHead(406);
                data = error(tr("Category could not be added"));
            }
        }
        else if (method == "removeCategory") {
            if (Database::instance()->removeCategory(query.queryItemValue("name"))) {
                response->writeHead(200);
                data = success();
            }
            else {
                response->writeHead(406);
                data = error(tr("Category could not be removed"));
            }
        }
        else if (method == "getCategoryPath") {
            QString path = Database::instance()->getCategoryPath(query.queryItemValue("name"));

            if (path.isEmpty()) {
                response->writeHead(404);
                data = error(tr("Category not found"));
            }
            else {
                response->writeHead(200);
                data = path.toUtf8();
            }
        }
    }
    else if (path.mid(1).startsWith("serviceAccounts")) {
        QString method = path.section('/', -1);

        if (method == "addAccount") {
            if (Database::instance()->addAccount(query.queryItemValue("serviceName"), query.queryItemValue("username"), query.queryItemValue("password"))) {
                response->writeHead(200);
                data = "{ \"serviceName\": \"" + query.queryItemValue("serviceName").toUtf8() + "\", \"username\": \"" + query.queryItemValue("username").toUtf8() + "\", \"password\": \"" + query.queryItemValue("password").toUtf8() + "\" }";

		if (ServicePlugin *plugin = PluginManager::instance()->getServicePlugin(query.queryItemValue("serviceName"))) {
            	    plugin->login(query.queryItemValue("username"), query.queryItemValue("password"));
        	}
            }
            else {
                response->writeHead(406);
                data = error(tr("Account could not be added"));
            }
        }
        else if (method == "removeAccount") {
            if (Database::instance()->removeCategory(query.queryItemValue("serviceName"))) {
                response->writeHead(200);
                data = success();
            }
            else {
                response->writeHead(406);
                data = error(tr("Account could not be removed"));
            }
        }
        else if (method == "getAccount") {
            QPair<QString, QString> account = Database::instance()->getAccount(query.queryItemValue("serviceName"));

            if (account.first.isEmpty()) {
                response->writeHead(404);
                data = error(tr("Account not found"));
            }
            else {
                response->writeHead(200);
                data = "{ \"serviceName\": \"" + query.queryItemValue("serviceName").toUtf8() + "\", \"username\": \"" + account.first.toUtf8() + "\", \"password\": \"" + account.second.toUtf8() + "\" }";
            }
        }
    }
    else if (path.mid(1).startsWith("decaptchaAccounts")) {
        QString method = path.section('/', -1);

        if (method == "addAccount") {
            if (Database::instance()->addAccount(query.queryItemValue("serviceName"), query.queryItemValue("username"), query.queryItemValue("password"))) {
                response->writeHead(200);
                data = "{ \"serviceName\": \"" + query.queryItemValue("serviceName").toUtf8() + "\", \"username\": \"" + query.queryItemValue("username").toUtf8() + "\", \"password\": \"" + query.queryItemValue("password").toUtf8() + "\" }";
            }
            else {
                response->writeHead(406);
                data = error(tr("Account could not be added"));
            }
        }
        else if (method == "removeAccount") {
            if (Database::instance()->removeCategory(query.queryItemValue("serviceName"))) {
                response->writeHead(200);
                data = success();
            }
            else {
                response->writeHead(406);
                data = error(tr("Account could not be removed"));
            }
        }
        else if (method == "getAccount") {
            QPair<QString, QString> account = Database::instance()->getAccount(query.queryItemValue("serviceName"));

            if (account.first.isEmpty()) {
                response->writeHead(404);
                data = error(tr("Account not found"));
            }
            else {
                response->writeHead(200);
                data = "{ \"serviceName\": \"" + query.queryItemValue("serviceName").toUtf8() + "\", \"username\": \"" + account.first.toUtf8() + "\", \"password\": \"" + account.second.toUtf8() + "\" }";
            }
        }
    }
    else {
        if (path == "/") {
            path += "queue";
        }

        data = resource(QFile::exists(path) ? path : QString("%1themes/%2%3").arg(m_path).arg(Settings::instance()->webInterfaceTheme()).arg(path));

        if (data.isEmpty()) {
            response->writeHead(404);
            data = error(tr("Not found"));
        }
        else {
            response->writeHead(200);
        }
    }
#else
    QUrl url = request->url();
    QString path = request->path();
    QByteArray data;

    if (!this->isAllowed(QHostAddress(request->remoteAddress()))) {
        response->writeHead(403);
        data = error(tr("Forbidden"));
    }
    else if (path == "/status") {
        response->writeHead(200);
        data = status();
    }
    else if (path == "/transfers") {
        response->writeHead(200);
        data = transfers(url.queryItemValue("filter"), url.queryItemValue("query"), url.queryItemValue("start").toInt(), url.queryItemValue("limit").toInt());
    }
    else if (path == "/categoryNames") {
        response->writeHead(200);
        data = categoryNames();
    }
    else if (path == "/categories") {
        response->writeHead(200);
        data = categories();
    }
    else if (path == "/serviceNames") {
        response->writeHead(200);
        data = serviceNames();
    }
    else if (path == "/serviceAccounts") {
        response->writeHead(200);
        data = serviceAccounts();
    }
    else if (path == "/decaptchaAccounts") {
        response->writeHead(200);
        data = decaptchaAccounts();
    }
    else if (path.mid(1).startsWith("urls")) {
        QString method = path.section('/', -1);

        if (method == "addUrls") {
            QStringList urls = url.queryItemValue("urls").split(',', QString::SkipEmptyParts);

            if (!urls.isEmpty()) {
                if (url.hasQueryItem("category")) {
                    Settings::instance()->setDefaultCategory(url.queryItemValue("category"));
                }

                UrlChecker::instance()->addUrlsToQueue(urls, url.queryItemValue("service"));
                response->writeHead(200);
                data = urlCheckProgress();
            }
            else {
                response->writeHead(406);
                data = error(tr("No URLs specified"));
            }
        }
        else if (method == "retrieveUrls") {
            QStringList urls = url.queryItemValue("urls").split(',', QString::SkipEmptyParts);

            if (!urls.isEmpty()) {
                UrlRetriever::instance()->addUrlsToQueue(urls);
                response->writeHead(200);
                data = urlRetrievalProgress();
            }
            else {
                response->writeHead(406);
                data = error(tr("No URLs specified"));
            }
        }
        else if (method == "cancelUrlChecks") {
            UrlChecker::instance()->cancel();
            response->writeHead(200);
            data = success();
        }
        else if (method == "clearUrlChecks") {
            UrlChecker::instance()->model()->clear();
            response->writeHead(200);
            data = success();
        }
        else if (method == "cancelUrlRetrieval") {
            UrlRetriever::instance()->cancel();
            response->writeHead(200);
            data = success();
        }
        else if (method == "clearRetrievedUrls") {
            UrlRetriever::instance()->clearResults();
            response->writeHead(200);
            data = success();
        }
        else if (method == "urlCheckProgress") {
            response->writeHead(200);
            data = urlCheckProgress();
        }
        else if (method == "urlRetrievalProgress") {
            response->writeHead(200);
            data = urlRetrievalProgress();
        }
    }
    else if (path.mid(1).startsWith("transfers")) {
        QString method = path.section('/', -1);

        if (method == "start") {
            if (url.hasQueryItem("id")) {
                if (TransferModel::instance()->start(url.queryItemValue("id"))) {
                    response->writeHead(200);
                    data = Json::serialize(TransferModel::instance()->itemData(url.queryItemValue("id")));
                }
                else {
                    response->writeHead(404);
                    data = error(tr("Transfer not found"));
                }
            }
            else {
                TransferModel::instance()->start();
                response->writeHead(200);
                data = success();
            }
        }
        else if (method == "pause") {
            if (url.hasQueryItem("id")) {
                if (TransferModel::instance()->pause(url.queryItemValue("id"))) {
                    response->writeHead(200);
                    data = Json::serialize(TransferModel::instance()->itemData(url.queryItemValue("id")));
                }
                else {
                    response->writeHead(404);
                    data = error(tr("Transfer not found"));
                }
            }
            else {
                TransferModel::instance()->pause();
                response->writeHead(200);
                data = success();
            }
        }
        else if ((method == "cancel") || (method == "remove")) {
            if (TransferModel::instance()->cancel(url.queryItemValue("id"))) {
                response->writeHead(200);
                data = success();
            }
            else {
                response->writeHead(404);
                data = error(tr("Transfer not found"));
            }
        }
        else if (method == "getTransferProperty") {
            QVariant variant = TransferModel::instance()->data(url.queryItemValue("id"), url.queryItemValue("property").toUtf8());

            if (!variant.isNull()) {
                response->writeHead(200);

                switch (variant.type()) {
                case QVariant::String:
                    data = variant.toString().toUtf8();
                    break;
                default:
                    data = Json::serialize(variant);
                }
            }
            else {
                response->writeHead(404);
                data = error(tr("Transfer not found"));
            }
        }
        else if (method == "setTransferProperty") {
            if (TransferModel::instance()->setData(url.queryItemValue("id"), url.queryItemValue("value"), url.queryItemValue("property").toUtf8())) {
                response->writeHead(200);
                data = Json::serialize(TransferModel::instance()->itemData(url.queryItemValue("id")));
            }
            else {
                response->writeHead(406);
                data = error(tr("Transfer property could not be set"));
            }
        }
        else if (method == "getProperty") {
            QVariant variant = TransferModel::instance()->property(url.queryItemValue("property").toUtf8());

            if (!variant.isNull()) {
                response->writeHead(200);

                switch (variant.type()) {
                case QVariant::String:
                    data = variant.toString().toUtf8();
                    break;
                default:
                    data = Json::serialize(variant);
                }
            }
            else {
                response->writeHead(404);
                data = error(tr("Property not found"));
            }
        }
        else if (method == "setProperty") {
            if (TransferModel::instance()->setProperty(url.queryItemValue("property").toUtf8(), url.queryItemValue("value"))) {
                response->writeHead(200);
                data = success();
            }
            else {
                response->writeHead(406);
                data = error(tr("Property could not be set"));
            }
        }
        else if (method == "quit") {
            response->writeHead(200);
            data = success();
            TransferModel::instance()->pause();
            QTimer::singleShot(1000, QCoreApplication::instance(), SLOT(quit()));
        }
        else {
            response->writeHead(404);
            data = error(tr("Method '%1' not found").arg(method));
        }
    }
    else if (path.mid(1).startsWith("preferences")) {
        QString method = path.section('/', -1);

        if (method == "getProperties") {
            QStringList properties = url.queryItemValue("properties").split(",", QString::SkipEmptyParts);

            if (!properties.isEmpty()) {
                response->writeHead(200);

                QVariantMap map;

                foreach (QString property, properties) {
                    map[property] = Settings::instance()->property(property.toUtf8());
                }

                data = Json::serialize(map);
            }
            else {
                response->writeHead(404);
                data = error(tr("No properties specified"));
            }
        }
        else if (method == "setProperties") {
            QList< QPair<QString, QString> > queryItems = url.queryItems();

            int head = queryItems.isEmpty() ? 406 : 200;

            while (!queryItems.isEmpty()) {
                QPair<QString, QString> queryItem = queryItems.takeFirst();

                if (!Settings::instance()->setProperty(queryItem.first.toUtf8(), queryItem.second)) {
                    head = 406;
                }
            }

            data = (head == 200 ? success() : error(tr("Some properties could not be set")));
        }
        else {
            response->writeHead(404);
            data = error(tr("Method '%1' not found").arg(method));
        }
    }
    else if (path.mid(1).startsWith("categories")) {
        QString method = path.section('/', -1);

        if (method == "addCategory") {
            if (Database::instance()->addCategory(url.queryItemValue("name"), url.queryItemValue("path"))) {
                response->writeHead(200);
                data = "{ \"name\": \"" + url.queryItemValue("name").toUtf8() + "\", \"path\": \"" + url.queryItemValue("path").toUtf8() + "\" }";
            }
            else {
                response->writeHead(406);
                data = error(tr("Category could not be added"));
            }
        }
        else if (method == "removeCategory") {
            if (Database::instance()->removeCategory(url.queryItemValue("name"))) {
                response->writeHead(200);
                data = success();
            }
            else {
                response->writeHead(406);
                data = error(tr("Category could not be removed"));
            }
        }
        else if (method == "getCategoryPath") {
            QString path = Database::instance()->getCategoryPath(url.queryItemValue("name"));

            if (path.isEmpty()) {
                response->writeHead(404);
                data = error(tr("Category not found"));
            }
            else {
                response->writeHead(200);
                data = path.toUtf8();
            }
        }
    }
    else if (path.mid(1).startsWith("serviceAccounts")) {
        QString method = path.section('/', -1);

        if (method == "addAccount") {
            if (Database::instance()->addAccount(url.queryItemValue("serviceName"), url.queryItemValue("username"), url.queryItemValue("password"))) {
                response->writeHead(200);
                data = "{ \"serviceName\": \"" + url.queryItemValue("serviceName").toUtf8() + "\", \"username\": \"" + url.queryItemValue("username").toUtf8() + "\", \"password\": \"" + url.queryItemValue("password").toUtf8() + "\" }";

		if (ServicePlugin *plugin = PluginManager::instance()->getServicePlugin(url.queryItemValue("serviceName"))) {
            	    plugin->login(url.queryItemValue("username"), url.queryItemValue("password"));
        	}
            }
            else {
                response->writeHead(406);
                data = error(tr("Account could not be added"));
            }
        }
        else if (method == "removeAccount") {
            if (Database::instance()->removeCategory(url.queryItemValue("serviceName"))) {
                response->writeHead(200);
                data = success();
            }
            else {
                response->writeHead(406);
                data = error(tr("Account could not be removed"));
            }
        }
        else if (method == "getAccount") {
            QPair<QString, QString> account = Database::instance()->getAccount(url.queryItemValue("serviceName"));

            if (account.first.isEmpty()) {
                response->writeHead(404);
                data = error(tr("Account not found"));
            }
            else {
                response->writeHead(200);
                data = "{ \"serviceName\": \"" + url.queryItemValue("serviceName").toUtf8() + "\", \"username\": \"" + account.first.toUtf8() + "\", \"password\": \"" + account.second.toUtf8() + "\" }";
            }
        }
    }
    else if (path.mid(1).startsWith("decaptchaAccounts")) {
        QString method = path.section('/', -1);

        if (method == "addAccount") {
            if (Database::instance()->addAccount(url.queryItemValue("serviceName"), url.queryItemValue("username"), url.queryItemValue("password"))) {
                response->writeHead(200);
                data = "{ \"serviceName\": \"" + url.queryItemValue("serviceName").toUtf8() + "\", \"username\": \"" + url.queryItemValue("username").toUtf8() + "\", \"password\": \"" + url.queryItemValue("password").toUtf8() + "\" }";
            }
            else {
                response->writeHead(406);
                data = error(tr("Account could not be added"));
            }
        }
        else if (method == "removeAccount") {
            if (Database::instance()->removeCategory(url.queryItemValue("serviceName"))) {
                response->writeHead(200);
                data = success();
            }
            else {
                response->writeHead(406);
                data = error(tr("Account could not be removed"));
            }
        }
        else if (method == "getAccount") {
            QPair<QString, QString> account = Database::instance()->getAccount(url.queryItemValue("serviceName"));

            if (account.first.isEmpty()) {
                response->writeHead(404);
                data = error(tr("Account not found"));
            }
            else {
                response->writeHead(200);
                data = "{ \"serviceName\": \"" + url.queryItemValue("serviceName").toUtf8() + "\", \"username\": \"" + account.first.toUtf8() + "\", \"password\": \"" + account.second.toUtf8() + "\" }";
            }
        }
    }
    else {
        if (path == "/") {
            path += "queue";
        }

        data = resource(QFile::exists(path) ? path : QString("%1themes/%2%3").arg(m_path).arg(Settings::instance()->webInterfaceTheme()).arg(path));

        if (data.isEmpty()) {
            response->writeHead(404);
            data = error(tr("Not found"));
        }
        else {
            response->writeHead(200);
        }
    }
#endif
    response->setHeader("Content-Type", "application/json");
    response->setHeader("Content-Length", QString::number(data.size()));
    response->end(data);
}
