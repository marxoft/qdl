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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "enums.h"
#include "file.h"
#include <QNetworkRequest>

class NetworkAccessManager;
class QNetworkReply;

class Connection : public QObject
{
    Q_OBJECT

public:
    explicit Connection(NetworkAccessManager *manager, QObject *parent = 0);
    ~Connection();

    QNetworkRequest request() const;
    void setRequest(const QNetworkRequest &request);

    QByteArray header(const QByteArray &headerName) const;
    void setHeader(const QByteArray &headerName, const QByteArray &value);

    qint64 contentRangeStart() const;
    void setContentRangeStart(qint64 start);

    qint64 contentRangeEnd() const;
    void setContentRangeEnd(qint64 end);

    void setContentRange(qint64 start, qint64 end);

    qint64 position() const;

    QByteArray data() const;
    void setData(const QByteArray &data);

    Transfers::Status status() const;
    QString errorString() const;

public slots:
    void start();
    void pause();
    void cancel();
    void processData();

private:
    void setStatus(Transfers::Status status);
    void setErrorString(const QString &errorString);

private slots:
    void performDownload();
    void followRedirect(const QUrl &url);
    void retry(const QUrl &url);
    void onMetaDataChanged();
    void onReadyRead();
    void onFinished();
    void onDownloadRateLimitChanged(int limit);

signals:
    void dataAvailable(qint64 offset, const QByteArray &data);
    void bytesDownloaded(qint64 bytes);
    void metaInfoReady(MetaInfo info);
    void statusChanged(Transfers::Status status);
    
private:
    NetworkAccessManager *m_nam;
    QNetworkReply *m_reply;
    QNetworkRequest m_request;
    qint64 m_start;
    qint64 m_end;
    qint64 m_downloadedBytes;
    QByteArray m_buffer;
    QByteArray m_data;
    Transfers::Status m_status;
    QString m_errorString;
    int m_redirects;
    int m_retries;
};

#endif // CONNECTION_H
