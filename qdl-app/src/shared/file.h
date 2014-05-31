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

#ifndef FILE_H
#define FILE_H

#include <QList>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QFile>

class QByteArray;

struct MetaInfo {
    qint64 size;
    qint64 bytesRemaining;
    QString name;
    QString path;
};

class File : public QThread
{
    Q_OBJECT

public:
    File(QObject *parent = 0);
    virtual ~File();

    void setMetaInfo(const MetaInfo &info);

    void write(qint64 offset, const QByteArray &data);
    bool remove();

    QString fileName() const;
    qint64 size() const;

    QString errorString() const;

protected:
    void run();

private:
    bool generateFiles();
    bool writeBlock(qint64 offset, const QByteArray &data);
    void setErrorString(const QString &errorString);

private slots:
    void wakeUp();

signals:
    void error();
    void writeCompleted();

private:
    struct WriteRequest {
        qint64 offset;
        QByteArray data;
    };

    QFile m_file;
    MetaInfo m_metaInfo;
    QString m_errString;

    bool m_newFile;
    bool m_writeOnly;
    bool m_quit;
    bool m_wokeUp;

    qint64 m_bytesRemaining;
    QList<WriteRequest> m_writeRequests;

    mutable QMutex m_mutex;
    mutable QWaitCondition m_cond;
};

#endif
