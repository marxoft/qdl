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

#include "connection.h"
#include "networkaccessmanager.h"
#include "settings.h"
#include <QNetworkReply>
#include <QDir>

static const qint64 BUFFER_SIZE = 1024 * 100;
static const int MAX_REDIRECTS = 8;
static const int MAX_RETRIES = 8;

Connection::Connection(NetworkAccessManager *manager, QObject *parent) :
    QObject(parent),
    m_nam(manager),
    m_reply(0),
    m_start(0),
    m_end(0),
    m_downloadedBytes(0),
    m_status(Transfers::Queued),
    m_redirects(0)
{
    this->connect(Settings::instance(), SIGNAL(downloadRateLimitChanged(int)), this, SLOT(onDownloadRateLimitChanged(int)));
}

Connection::~Connection() {}

QNetworkRequest Connection::request() const {
    return m_request;
}

void Connection::setRequest(const QNetworkRequest &request) {
    m_request = request;
}

QByteArray Connection::header(const QByteArray &headerName) const {
    return m_request.rawHeader(headerName);
}

void Connection::setHeader(const QByteArray &headerName, const QByteArray &value) {
    m_request.setRawHeader(headerName, value);
}

qint64 Connection::contentRangeStart() const {
    return m_start;
}

void Connection::setContentRangeStart(qint64 start) {
    m_start = start;
}

qint64 Connection::contentRangeEnd() const {
    return m_end;
}

void Connection::setContentRangeEnd(qint64 end) {
    m_end = end;
}

void Connection::setContentRange(qint64 start, qint64 end) {
    this->setContentRangeStart(start);
    this->setContentRangeEnd(end);
}

qint64 Connection::position() const {
    return m_start + m_downloadedBytes;
}

QByteArray Connection::data() const {
    return m_data;
}

void Connection::setData(const QByteArray &data) {
    m_data = data;
}

Transfers::Status Connection::status() const {
    return m_status;
}

void Connection::setStatus(Transfers::Status status) {
    if (status != this->status()) {
        m_status = status;
        emit statusChanged(status);
    }
}

QString Connection::errorString() const {
    return m_errorString;
}

void Connection::setErrorString(const QString &errorString) {
    m_errorString = errorString;
}

void Connection::start() {
    if (this->position() > 0) {
        this->setHeader("Range", "bytes=" + QByteArray::number(this->position()) + "-");
    }

    this->performDownload();
}

void Connection::performDownload() {
    this->setStatus(Transfers::Downloading);

    qDebug() << "Downloading:" << this->request().url();
    
    if (m_reply) {
        delete m_reply;
    }

    m_redirects = 0;
    m_retries = 0;
    m_reply = this->data().isEmpty() ? m_nam->get(this->request()) : m_nam->post(this->request(), this->data());
    this->connect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));

    if (this->contentRangeEnd() <= 0) {
        this->connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onMetaDataChanged()));
    }
    else {
        this->connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    }
}

void Connection::followRedirect(const QUrl &url) {
    qDebug() << "Following redirect:" << url;
    
    if (m_reply) {
        delete m_reply;
    }

    m_redirects++;
    
    QNetworkRequest request(url);
    
    if (this->position() > 0) {
        request.setRawHeader("Range", "bytes=" + QByteArray::number(this->position()) + "-");
    }
    
    m_reply = m_nam->get(request);
    
    this->connect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));

    if (this->contentRangeEnd() <= 0) {
        this->connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onMetaDataChanged()));
    }
    else {
        this->connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    }
}

void Connection::retry(const QUrl &url) {
    qDebug() << "Retrying:" << url;
    
    if (m_reply) {
        delete m_reply;
    }

    m_retries++;
    
    QNetworkRequest request(url);
    
    if (this->position() > 0) {
        request.setRawHeader("Range", "bytes=" + QByteArray::number(this->position()) + "-");
    }
    
    m_reply = m_nam->get(request);
    this->connect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));

    if (this->contentRangeEnd() <= 0) {
        this->connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onMetaDataChanged()));
    }
    else {
        this->connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    }
}

void Connection::pause() {
    if (m_reply) {
        m_reply->abort();
    }

    this->setStatus(Transfers::Paused);
}

void Connection::cancel() {
    if (m_reply) {
        m_reply->abort();
    }

    this->setStatus(Transfers::Canceled);
}

void Connection::processData() {
    if (m_reply) {
        this->connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    }
}

void Connection::onMetaDataChanged() {
    if (m_reply) {
        qint64 size = m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();

        if (size <= 0) {
            size = m_reply->rawHeader("Content-Length").toLongLong();
        }

        if (size <= 0) {
            QString redirect = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

            if (!redirect.isEmpty()) {
                return;
            }

            redirect = m_reply->header(QNetworkRequest::LocationHeader).toString();

            if (!redirect.isEmpty()) {
                return;
            }
        }

        qDebug() << "Reported size:" << size;
        this->setContentRangeEnd(size);

        QString fileName = QString(m_reply->rawHeader("Content-Disposition")).section("=", -1)
                                                                             .section(QRegExp("[\"']"), 1, 1)
                                                                             .remove(QRegExp("[\"';]"));

        MetaInfo info;
        info.size = size;
        info.bytesRemaining = size;
        info.name = fileName;

        emit metaInfoReady(info);
    }
}

void Connection::onReadyRead() {
    if (m_reply) {
        if (this->contentRangeEnd() > 0) {
            qint64 maxBytes = qMin<qint64>(this->contentRangeEnd() - (this->position() + m_buffer.size()),
                                           m_reply->bytesAvailable());

            m_buffer += m_reply->read(maxBytes);
            qint64 bufferSize = qint64(m_buffer.size());

            if (bufferSize > BUFFER_SIZE) {
                emit bytesDownloaded(bufferSize);
                emit dataAvailable(this->position(), m_buffer);

                m_downloadedBytes += bufferSize;
                m_buffer.clear();
            }

            if (this->position() >= this->contentRangeEnd()) {
                m_reply->abort();
            }
        }
        else {
            qint64 maxBytes = m_reply->bytesAvailable();

            m_buffer += m_reply->read(maxBytes);
            qint64 bufferSize = qint64(m_buffer.size());

            if (bufferSize > BUFFER_SIZE) {
                emit bytesDownloaded(bufferSize);
                emit dataAvailable(this->position(), m_buffer);

                m_downloadedBytes += bufferSize;
                m_buffer.clear();
            }
        }
    }
}

void Connection::onFinished() {
    if (!m_reply) {
        this->setErrorString(tr("Network error"));
        this->setStatus(Transfers::Failed);
        return;
    }

    QUrl redirect = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (redirect.isEmpty()) {
        redirect = m_reply->header(QNetworkRequest::LocationHeader).toUrl();
    }

    if (!redirect.isEmpty()) {
        m_reply->deleteLater();
        m_reply = 0;
        
        if (m_redirects < MAX_REDIRECTS) {
            this->followRedirect(redirect);
        }
        else {
            this->setErrorString(tr("Maximum redirects reached"));
            this->setStatus(Transfers::Failed);
        }
        
        return;
    }

    switch (m_reply->error()) {
    case QNetworkReply::NoError:
        if (!m_buffer.isEmpty()) {
            qint64 bytes = qint64(m_buffer.size());

            emit bytesDownloaded(bytes);
            emit dataAvailable(this->position(), m_buffer);

            m_downloadedBytes += bytes;
            m_buffer.clear();
        }
        
        this->setStatus(Transfers::Completed);
        break;
    case QNetworkReply::OperationCanceledError:
        m_buffer.clear();
        
        if ((this->contentRangeEnd() > 0) && (this->position() >= this->contentRangeEnd())) {
            this->setStatus(Transfers::Completed);
        }

        break;
    case QNetworkReply::RemoteHostClosedError:
        m_buffer.clear();
    
        if ((m_retries < MAX_RETRIES) && (!m_reply->url().isEmpty())) {
            redirect = m_reply->url();
            m_reply->deleteLater();
            m_reply = 0;
            this->retry(redirect);
            return;
        }
        else {
            qDebug() << "Connection error:" << m_reply->error() << m_reply->errorString();
            this->setErrorString(m_reply->errorString());
            this->setStatus(Transfers::Failed);
        }
        
        break;
    default:
        m_buffer.clear();
        qDebug() << "Connection error:" << m_reply->error() << m_reply->errorString();
        this->setErrorString(m_reply->errorString());
        this->setStatus(Transfers::Failed);
        break;
    }
    
    m_reply->deleteLater();
    m_reply = 0;
}

void Connection::onDownloadRateLimitChanged(int limit) {
    if (m_reply) {
        m_reply->setReadBufferSize(limit * 4);
    }
}
