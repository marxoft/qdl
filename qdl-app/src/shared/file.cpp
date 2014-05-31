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

#include "file.h"
#include <QByteArray>
#include <QDir>
#include <QDebug>

File::File(QObject *parent)
    : QThread(parent),
      m_newFile(false),
      m_writeOnly(false),
      m_quit(false),
      m_wokeUp(false),
      m_bytesRemaining(0)
{
}

File::~File() {
    m_quit = true;
    m_cond.wakeOne();
    this->wait();

    m_file.close();
    this->quit();
}

void File::setMetaInfo(const MetaInfo &info) {
    m_metaInfo = info;
    m_bytesRemaining = info.bytesRemaining;
    m_writeOnly = (info.size <= 0);
}

void File::write(qint64 offset, const QByteArray &data) {
    WriteRequest request;
    request.offset = offset;
    request.data = data;

    QMutexLocker locker(&m_mutex);
    m_writeRequests << request;

    if (!m_wokeUp) {
        m_wokeUp = true;
        QMetaObject::invokeMethod(this, "wakeUp", Qt::QueuedConnection);
    }
}

bool File::remove() {
    m_writeRequests.clear();

    return this->fileName().isEmpty() ? false : m_file.remove();
}

QString File::fileName() const {
    return m_file.fileName();
}

qint64 File::size() const {
    return m_file.size();
}

QString File::errorString() const {
    return m_errString;
}

void File::setErrorString(const QString &errorString) {
    qDebug() << errorString;
    m_errString = errorString;
}

void File::run() {
    if (!this->generateFiles()) {
        return;
    }

    do {
        {
            // Go to sleep if there's nothing to do.
            QMutexLocker locker(&m_mutex);

            if ((!m_quit) && (m_writeRequests.isEmpty())) {
                m_cond.wait(&m_mutex);
            }
        }

        // Write pending write requests
        m_mutex.lock();
        QList<WriteRequest> newWriteRequests = m_writeRequests;
        m_writeRequests.clear();

        while ((!m_quit) && (!newWriteRequests.isEmpty())) {
            WriteRequest request = newWriteRequests.takeFirst();
            this->writeBlock(request.offset, request.data);
        }

        m_mutex.unlock();

    } while (!m_quit);

    // Write pending write requests
    m_mutex.lock();
    QList<WriteRequest> newWriteRequests = m_writeRequests;
    m_writeRequests.clear();
    m_mutex.unlock();

    while (!newWriteRequests.isEmpty()) {
        WriteRequest request = newWriteRequests.takeFirst();
        this->writeBlock(request.offset, request.data);
    }

    m_file.close();
}

bool File::generateFiles() {
    QMutexLocker locker(&m_mutex);
    QString prefix;

    if (!m_metaInfo.path.isEmpty()) {
        prefix = m_metaInfo.path;

        if (!prefix.endsWith("/")) {
            prefix += "/";
        }

        QDir dir;

        if (!dir.mkpath(prefix)) {
            this->setErrorString(tr("Failed to create directory %1").arg(prefix));
            emit error();

            return false;
        }
    }

    m_file.setFileName(prefix + m_metaInfo.name);

    if (m_writeOnly) {
        if (!m_file.open(m_file.exists() ? QFile::Append : QFile::WriteOnly)) {
            this->setErrorString(tr("Failed to open file %1: %2")
                    .arg(m_file.fileName()).arg(m_file.errorString()));

            emit error();

            return false;
        }
    }
    else {
        if (!m_file.open(QFile::ReadWrite)) {
            this->setErrorString(tr("Failed to open file %1: %2")
                    .arg(m_file.fileName()).arg(m_file.errorString()));

            emit error();

            return false;
        }

        if (m_file.size() != m_metaInfo.size) {
            m_newFile = true;

            if (!m_file.resize(m_metaInfo.size)) {
                this->setErrorString(tr("Failed to resize file %1: %2")
                        .arg(m_file.fileName()).arg(m_file.errorString()));

                m_file.close();

                emit error();

                return false;
            }
        }
    }

    m_file.close();

    return true;
}

bool File::writeBlock(qint64 offset, const QByteArray &data) {
    if (!m_quit) {
        if (m_writeOnly) {
            if (!m_file.isOpen()) {
                if (!m_file.open(QFile::Append)) {
                    this->setErrorString(tr("Failed to write to file %1: %2")
                            .arg(m_file.fileName()).arg(m_file.errorString()));

                    emit error();

                    return false;
                }
            }

            qint64 bytesWritten = m_file.write(data);

            if (bytesWritten <= 0) {
                this->setErrorString(tr("Failed to write to file %1: %2")
                        .arg(m_file.fileName()).arg(m_file.errorString()));

                emit error();

                return false;
            }
        }
        else if (m_bytesRemaining > 0) {
            if (!m_file.isOpen()) {
                if (!m_file.open(QFile::ReadWrite)) {
                    this->setErrorString(tr("Failed to write to file %1: %2")
                            .arg(m_file.fileName()).arg(m_file.errorString()));

                    emit error();

                    return false;
                }
            }

            m_file.seek(offset);
            qint64 bytesWritten = m_file.write(data.constData(),
                                               qMin<qint64>(data.size(), m_file.size() - m_file.pos()));

            if (bytesWritten <= 0) {
                this->setErrorString(tr("Failed to write to file %1: %2")
                        .arg(m_file.fileName()).arg(m_file.errorString()));

                emit error();

                return false;
            }

            m_bytesRemaining -= bytesWritten;

            if (m_bytesRemaining == 0) {
                m_file.close();
                emit writeCompleted();
            }
        }
    }

    return true;
}

void File::wakeUp() {
    QMutexLocker locker(&m_mutex);
    m_wokeUp = false;
    m_cond.wakeOne();
}
