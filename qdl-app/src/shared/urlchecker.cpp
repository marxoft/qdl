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

#include "urlchecker.h"
#include "utils.h"
#include "pluginmanager.h"
#include "networkaccessmanager.h"
#include "../interfaces/serviceplugin.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QIcon>

UrlChecker* UrlChecker::self = 0;

UrlChecker::UrlChecker() :
    QObject(),
    m_model(new UrlCheckModel(this)),
    m_index(0),
    m_canceled(false)
{
    if (!self) {
        self = this;
    }

    this->connect(PluginManager::instance(), SIGNAL(pluginsReady()), this, SLOT(connectToPluginSignals()));
    this->connectToPluginSignals();
}

UrlChecker::~UrlChecker() {}

UrlChecker* UrlChecker::instance() {
    return !self ? new UrlChecker : self;
}

UrlCheckModel* UrlChecker::model() {
    return m_model;
}

void UrlChecker::connectToPluginSignals() {
    foreach (ServicePlugin *plugin, PluginManager::instance()->servicePlugins()) {
        this->connect(plugin, SIGNAL(urlChecked(bool,QUrl,QString,QString,bool)), this, SLOT(onUrlChecked(bool,QUrl,QString,QString,bool)));
    }
}

void UrlChecker::checkUrl(const UrlCheck &check) {
    ServicePlugin *plugin = 0;

    if (check.service.isEmpty()) {
        plugin = PluginManager::instance()->getServicePlugin(check.url);
    }
    else {
        plugin = PluginManager::instance()->getServicePlugin(check.service);
    }

    if (plugin) {
        plugin->checkUrl(check.url);
    }
    else {
        this->testFileDownload(check.url);
    }
}

int UrlChecker::progress() const {
    return (!m_model->rowCount()) || (!m_index) ? 0 : m_index * 100 / m_model->rowCount();
}

void UrlChecker::testFileDownload(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = NetworkAccessManager::instance()->head(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkFileDownload()));
}

void UrlChecker::checkFileDownload() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        this->onUrlChecked(false, QUrl(), QString(), QString(), true);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toUrl();
    }

    if (!redirect.isEmpty()) {
        this->testFileDownload(redirect);
    }
    else {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        switch (statusCode) {
        case 200:
        case 201:
        case 206:
        {
            QString fileName = QString(reply->rawHeader("Content-Disposition")).section("filename=", -1).section(';', 0, 0).remove(QRegExp("\'|\""));

            if (fileName.isEmpty()) {
                fileName = reply->request().url().toString().section('/', -1);
            }

            if (fileName.isEmpty()) {
                this->onUrlChecked(false, QUrl(), QString(), QString(), true);
            }
            else {
                this->onUrlChecked(true, reply->request().url(), QString(), fileName, true);
            }

            break;
        }
        default:
            this->onUrlChecked(false, QUrl(), QString(), QString(), true);
            break;
        }
    }

    reply->deleteLater();
}

void UrlChecker::addUrlToQueue(const QUrl &url) {
    this->addUrlToQueue(url, QString());
}

void UrlChecker::addUrlToQueue(const QUrl &url, const QString &service) {
    m_canceled = false;
    bool start = m_urlQueue.isEmpty();

    if (m_model->rowCount() == 0) {
        m_index = 0;
    }

    UrlCheck check;
    check.url = url;
    check.service = service;
    check.checked = false;
    check.ok = false;
    m_urlQueue.enqueue(check);
    m_model->addUrlCheck(check);

    if (start) {
        emit progressChanged(this->progress());
        this->checkUrl(m_urlQueue.dequeue());
    }
}

void UrlChecker::addUrlToQueue(const QString &url) {
    this->addUrlToQueue(url, QString());
}

void UrlChecker::addUrlToQueue(const QString &url, const QString &service) {
    m_canceled = false;
    bool start = m_urlQueue.isEmpty();

    if (m_model->rowCount() == 0) {
        m_index = 0;
    }

    UrlCheck check;
    check.service = service;
    check.checked = false;
    check.ok = false;

#if QT_VERSION >= 0x040600
    check.url = QUrl::fromUserInput(url);
#else
    check.url = (url.startsWith("http") ? url : "http://" + url);
#endif
    m_urlQueue.enqueue(check);
    m_model->addUrlCheck(check);

    if (start) {
        emit progressChanged(this->progress());
        this->checkUrl(m_urlQueue.dequeue());
    }
}

void UrlChecker::addUrlsToQueue(QList<QUrl> urls) {
    this->addUrlsToQueue(urls, QString());
}

void UrlChecker::addUrlsToQueue(QList<QUrl> urls, const QString &service) {
    m_canceled = false;
    bool start = m_urlQueue.isEmpty();

    if (m_model->rowCount() == 0) {
        m_index = 0;
    }

    foreach (QUrl url, urls) {
        UrlCheck check;
        check.url = url;
        check.service = service;
        check.checked = false;
        check.ok = false;
        m_urlQueue.enqueue(check);
        m_model->addUrlCheck(check);
    }

    if (start) {
        emit progressChanged(this->progress());
        this->checkUrl(m_urlQueue.dequeue());
    }
}

void UrlChecker::addUrlsToQueue(QStringList urls) {
    this->addUrlsToQueue(urls, QString());
}

void UrlChecker::addUrlsToQueue(QStringList urls, const QString &service) {
    m_canceled = false;
    bool start = m_urlQueue.isEmpty();

    if (m_model->rowCount() == 0) {
        m_index = 0;
    }

    foreach (QString url, urls) {
        UrlCheck check;
        check.service = service;
        check.checked = false;
        check.ok = false;

#if QT_VERSION >= 0x040600
        check.url = QUrl::fromUserInput(url);
#else
        check.url = (url.startsWith("http") ? url : "http://" + url);
#endif
        m_urlQueue.enqueue(check);
        m_model->addUrlCheck(check);
    }

    if (start) {
        emit progressChanged(this->progress());
        this->checkUrl(m_urlQueue.dequeue());
    }
}

void UrlChecker::parseUrlsFromText(const QString &text) {
    this->parseUrlsFromText(text, QString());
}

void UrlChecker::parseUrlsFromText(const QString &text, const QString &service) {
    QStringList urlStrings = text.split(QRegExp("\\s"), QString::SkipEmptyParts);
    this->addUrlsToQueue(urlStrings, service);
}

void UrlChecker::importUrlsFromTextFile(const QString &filePath) {
    this->importUrlsFromTextFile(filePath, QString());
}

void UrlChecker::importUrlsFromTextFile(const QString &filePath, const QString &service) {
    QFile textFile(filePath);

    if (textFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString urlString = textFile.readAll();
        textFile.close();
        this->parseUrlsFromText(urlString, service);
    }
}

void UrlChecker::cancel() {
    if (!m_canceled) {
        m_canceled = true;

        const QModelIndex index = m_model->index(m_index, 0);

        if (index.isValid()) {
            QString service = index.data(UrlCheckModel::ServiceRole).toString();
            ServicePlugin *plugin = 0;

            if (service.isEmpty()) {
                plugin = PluginManager::instance()->getServicePlugin(index.data(UrlCheckModel::UrlRole).toUrl());
            }
            else {
                plugin = PluginManager::instance()->getServicePlugin(service);
            }

            if (plugin) {
                plugin->cancelCurrentOperation();
            }
        }

        m_urlQueue.clear();
        emit canceled();
    }
}

void UrlChecker::onUrlChecked(bool ok, const QUrl &url, const QString &service, const QString &fileName, bool done) {
    if (ok) {
        emit urlReady(url, service, fileName);
    }

    if (done) {
        m_model->urlChecked(m_index, ok);
        m_index++;
        emit progressChanged(this->progress());

        if ((!m_urlQueue.isEmpty()) && (!m_canceled)) {
            this->checkUrl(m_urlQueue.dequeue());
        }
    }
}

UrlCheckModel::UrlCheckModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    m_roleNames[UrlRole] = "url";
    m_roleNames[ServiceRole] = "service";
    m_roleNames[CheckedRole] = "checked";
    m_roleNames[OkRole] = "ok";
#if (QT_VERSION >= 0x040600) && (QT_VERSION < 0x050000)
    this->setRoleNames(m_roleNames);
#endif
}

UrlCheckModel::~UrlCheckModel() {}

#if (QT_VERSION >= 0x050000) || (QT_VERSION < 0x040600)
QHash<int, QByteArray> UrlCheckModel::roleNames() const {
    return m_roleNames;
}
#endif

int UrlCheckModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return m_list.size();
}

int UrlCheckModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return 2;
}

QVariant UrlCheckModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return tr("URL");
    case 1:
        return tr("OK?");
    default:
        return QVariant();
    }
}

QVariant UrlCheckModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case UrlRole:
        return m_list.at(index.row()).url;
    case ServiceRole:
        return m_list.at(index.row()).service;
    case CheckedRole:
        return m_list.at(index.row()).checked;
    case OkRole:
        return m_list.at(index.row()).ok;
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return m_list.at(index.row()).url;
        default:
            return QVariant();
        }
    case Qt::DecorationRole:
        switch (index.column()) {
        case 1:
#ifdef Q_WS_MAEMO_5
            return !m_list.at(index.row()).checked ? QVariant() : m_list.at(index.row()).ok ? QIcon::fromTheme("widgets_tickmark_list")
                                                                                            : QIcon::fromTheme("general_stop");
#elif defined MAEMO4_OS
            return !m_list.at(index.row()).checked ? QVariant() : m_list.at(index.row()).ok ? QIcon("/usr/share/icons/hicolor/26x26/hildon/qgn_list_gene_valid")
                                                                                            : QIcon("/usr/share/icons/hicolor/26x26/hildon/qgn_list_gene_invalid");
#else
            return !m_list.at(index.row()).checked ? QVariant() : m_list.at(index.row()).ok ? QIcon::fromTheme("dialog-apply")
                                                                                            : QIcon::fromTheme("dialog-error");
#endif
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}

QVariant UrlCheckModel::data(int row, const QByteArray &role) const {
    return this->data(this->index(row, 0), this->roleNames().key(role));
}

bool UrlCheckModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()) {
        return false;
    }

    switch (role) {
    case UrlRole:
        m_list[index.row()].url = value.toUrl();
        break;
    case ServiceRole:
        m_list[index.row()].service = value.toString();
        break;
    case CheckedRole:
        m_list[index.row()].checked = value.toBool();
        break;
    case OkRole:
        m_list[index.row()].ok = value.toBool();
        break;
    default:
        return false;
    }

    emit dataChanged(index, index);

    return true;
}

QMap<int, QVariant> UrlCheckModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    for (int role = UrlRole; role <= OkRole; role++) {
        map.insert(role, index.data(role));
    }

    return map;
}

QVariantMap UrlCheckModel::itemData(int row) const {
    QVariantMap map;

    for (int role = UrlRole; role <= OkRole; role++) {
        map.insert(this->roleNames().value(role), this->index(row, 0).data(role));
    }

    return map;
}

QVariantList UrlCheckModel::allItemData() const {
    QVariantList list;

    for (int i = 0; i < this->rowCount(); i++) {
        list.append(this->itemData(i));
    }

    return list;
}

void UrlCheckModel::addUrlCheck(const UrlCheck &check) {
    this->beginInsertRows(QModelIndex(), this->rowCount(), this->rowCount());
    m_list.append(check);
    this->endInsertRows();

    emit countChanged(this->rowCount());
}

void UrlCheckModel::urlChecked(const QUrl &url, bool ok) {
    for (int i = 0; i < m_list.size(); i++) {
        if (m_list.at(i).url == url) {
            this->urlChecked(i, ok);

            return;
        }
    }
}

void UrlCheckModel::urlChecked(int row, bool ok) {
    if ((row >= 0) && (row < this->rowCount())) {
        this->setData(this->index(row, 1), ok, OkRole);
        this->setData(this->index(row, 1), true, CheckedRole);
    }
}

void UrlCheckModel::removeUrlCheck(int row) {
    if ((row < 0) || (row >= this->rowCount())) {
        return;
    }

    this->beginRemoveRows(QModelIndex(), row, row);
    m_list.removeAt(row);
    this->endRemoveRows();

    emit countChanged(this->rowCount());
}

void UrlCheckModel::clear() {
    this->beginRemoveRows(QModelIndex(), 0, this->rowCount() - 1);
    m_list.clear();
    this->endRemoveRows();

    emit countChanged(this->rowCount());
}
