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

#ifndef URLRETRIEVER_H
#define URLRETRIEVER_H

#include <QObject>
#include <QQueue>
#include <QUrl>
#include <QStringList>

class QNetworkReply;
class UrlProcessor;

class UrlRetriever : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int progress
               READ progress
               NOTIFY progressChanged)

public:
    int progress() const;

    Q_INVOKABLE QList<QUrl> results() const;
    Q_INVOKABLE QString resultsString() const;

    static UrlRetriever* instance();

public slots:
    void addUrlToQueue(const QUrl &url);
    void addUrlToQueue(const QString &url);
    void addUrlsToQueue(QList<QUrl> urls);
    void addUrlsToQueue(QStringList urls);
    void parseUrlsFromText(const QString &text);
    void importUrlsFromTextFile(const QString &filePath);

    void cancel();
    void clearResults();

private:
    UrlRetriever();
    ~UrlRetriever();

    void getWebPage(const QUrl &url);

private slots:
    void checkWebPage();
    void onUrlsProcessed(const QList<QUrl> &urls);

signals:
    void busy(const QString &message, int numberOfOperations);
    void finished();
    void progressChanged(int progress);
    void canceled();
    
private:
    static UrlRetriever *self;

    UrlProcessor *m_processor;
    QNetworkReply *m_reply;

    QQueue<QUrl> m_urlQueue;
    QList<QUrl> m_results;

    int m_index;
    int m_total;
    bool m_canceled;
};

class UrlProcessor : public QObject
{
    Q_OBJECT

    friend class UrlRetriever;

private:
    UrlProcessor();
    ~UrlProcessor();

private slots:
    void processUrls(const QString &response);

signals:
    void urlsProcessed(const QList<QUrl> &urls);
};

#endif // URLRETRIEVER_H
