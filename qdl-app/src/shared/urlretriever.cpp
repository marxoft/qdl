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

#include "urlretriever.h"
#include "networkaccessmanager.h"
#include "pluginmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegExp>
#include <QFile>
#include <QThread>

UrlRetriever* UrlRetriever::self = 0;

UrlRetriever::UrlRetriever() :
    QObject(),
    m_processor(new UrlProcessor),
    m_reply(0),
    m_index(0),
    m_total(0),
    m_cancelled(false)
{
    if (!self) {
        self = this;
    }

    this->connect(m_processor, SIGNAL(urlsProcessed(QList<QUrl>)), this, SLOT(onUrlsProcessed(QList<QUrl>)));
}

UrlRetriever::~UrlRetriever() {
    m_processor->deleteLater();

    if (m_reply) {
        m_reply->deleteLater();
    }
}

UrlRetriever* UrlRetriever::instance() {
    return !self ? new UrlRetriever : self;
}

int UrlRetriever::progress() const {
    return (m_index <= 0) || (m_total <= 0) ? 0 : m_index * 100 / m_total;
}

void UrlRetriever::addUrlToQueue(const QUrl &url) {
    m_cancelled = false;
    m_urlQueue.enqueue(url);
    m_total = m_urlQueue.size();

    if (m_total == 1) {
	m_index = 0;
        emit busy(tr("Retrieving URLs"), 100);
        emit progressChanged(0);
        this->getWebPage(m_urlQueue.dequeue());
    }
}

void UrlRetriever::addUrlToQueue(const QString &url) {
    m_cancelled = false;
    m_urlQueue.enqueue(url);
    m_total = m_urlQueue.size();

    if (m_total == 1) {
	m_index = 0;
        emit busy(tr("Retrieving URLs"), 100);
        emit progressChanged(0);
        this->getWebPage(m_urlQueue.dequeue());
    }
}

void UrlRetriever::addUrlsToQueue(QList<QUrl> urls) {
    foreach (QUrl url, urls) {
        this->addUrlToQueue(url);
    }
}

void UrlRetriever::addUrlsToQueue(QStringList urls) {
    foreach (QString url, urls) {
        this->addUrlToQueue(url);
    }
}

void UrlRetriever::parseUrlsFromText(const QString &text) {
    QStringList urlStrings = text.split(QRegExp("\\s"), QString::SkipEmptyParts);
    this->addUrlsToQueue(urlStrings);
}

void UrlRetriever::importUrlsFromTextFile(const QString &filePath) {
    QFile textFile(filePath);

    if (textFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString urlString = textFile.readAll();
        textFile.close();
        this->parseUrlsFromText(urlString);
    }
}

void UrlRetriever::cancel() {
    if (!m_cancelled) {
        m_cancelled = true;

        if (m_reply) {
            m_reply->deleteLater();
            m_reply = 0;
        }

        m_urlQueue.clear();

        emit cancelled();
    }
}

void UrlRetriever::clearResults() {
    m_results.clear();
}

void UrlRetriever::getWebPage(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.22 (KHTML, like Gecko) Ubuntu Chromium/25.0.1364.160 Chrome/25.0.1364.160 Safari/537.22");
    m_reply = NetworkAccessManager::instance()->get(request);
    this->connect(m_reply, SIGNAL(finished()), this, SLOT(checkWebPage()));
}

void UrlRetriever::checkWebPage() {
    if (!m_reply) {
        if (!m_cancelled) {
            this->cancel();
        }

        return;
    }

    QUrl redirect = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (redirect.isEmpty()) {
        redirect = m_reply->header(QNetworkRequest::LocationHeader).toUrl();
    }

    if (!redirect.isEmpty()) {
        if (redirect.scheme().isEmpty()) {
            redirect.setScheme(m_reply->request().url().scheme());
        }

        if (redirect.host().isEmpty()) {
            redirect.setHost(m_reply->request().url().host());
        }

        m_reply->deleteLater();
        m_reply = 0;
        this->getWebPage(redirect);
        return;
    }

    QString response(m_reply->readAll());
    QMetaObject::invokeMethod(m_processor, "processUrls", Qt::QueuedConnection, Q_ARG(QString, response));

    m_reply->deleteLater();
    m_reply = 0;
}

void UrlRetriever::onUrlsProcessed(const QList<QUrl> &urls) {
    m_results.append(urls);

    m_index++;
    emit progressChanged(this->progress());

    if ((!m_urlQueue.isEmpty()) && (!m_cancelled)) {
        this->getWebPage(m_urlQueue.dequeue());
    }
    else {
        emit finished();
    }
}

QList<QUrl> UrlRetriever::results() const {
    return m_results;
}

QString UrlRetriever::resultsString() const {
    QString string;

    foreach (QUrl url, m_results) {
        string.append(url.toString() + "\n");
    }

    return string;
}

UrlProcessor::UrlProcessor() :
    QObject()
{
    this->moveToThread(new QThread);
    this->thread()->start(QThread::LowestPriority);
}

UrlProcessor::~UrlProcessor() {
    this->thread()->quit();
    this->thread()->deleteLater();
}

void UrlProcessor::processUrls(const QString &response) {
    QList<QUrl> urls;

    QRegExp re("http(s|)://[^'\"<\\s]+");
    int pos = 0;

    while ((pos = re.indexIn(response, pos)) != -1) {
        QUrl url(re.cap(0));

        if ((PluginManager::instance()->servicePluginExists(url)) && (!urls.contains(url))) {
            urls.append(url);
        }

        pos += re.matchedLength();
    }

    emit urlsProcessed(urls);
}
