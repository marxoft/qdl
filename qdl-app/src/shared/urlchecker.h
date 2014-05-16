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

#ifndef URLCHECKER_H
#define URLCHECKER_H

#include <QObject>
#include <QUrl>
#include <QQueue>
#include <QStringList>
#include <QAbstractTableModel>

class UrlCheckModel;

class UrlChecker : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int progress
               READ progress
               NOTIFY progressChanged)

public:
    UrlCheckModel* model();

    int progress() const;

    static UrlChecker* instance();

public slots:
    void addUrlToQueue(const QUrl &url);
    void addUrlToQueue(const QString &url);
    void addUrlsToQueue(QList<QUrl> urls);
    void addUrlsToQueue(QStringList urls);
    void parseUrlsFromText(const QString &text);
    void importUrlsFromTextFile(const QString &filePath);

    void cancel();

private:
    UrlChecker();
    ~UrlChecker();

    void testFileDownload(const QUrl &url);

private slots:
    void connectToPluginSignals();
    void checkUrl(const QUrl &url);
    void checkFileDownload();
    void onUrlChecked(bool ok, const QUrl &url, const QString &service, const QString &fileName, bool done);

signals:
    void urlReady(const QUrl &url, const QString &service, const QString &fileName);
    void progressChanged(int progress);
    void cancelled();
    
private:
    static UrlChecker *self;

    UrlCheckModel* m_model;

    QQueue<QUrl> m_urlQueue;

    int m_index;
    bool m_cancelled;
};

typedef struct {
    QString url;
    bool checked;
    bool ok;
} UrlCheck;

class UrlCheckModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(int count
               READ rowCount
               NOTIFY countChanged)

    friend class UrlChecker;

public:
    enum Roles {
        UrlRole = Qt::UserRole + 1,
        CheckedRole,
        OkRole
    };

#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
#if QT_VERSION >= 0x040600
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
#endif
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    QVariantMap itemData(int row) const;
    Q_INVOKABLE QVariantList allItemData() const;

public slots:
    void clear();

private:
    explicit UrlCheckModel(QObject *parent = 0);
    ~UrlCheckModel();

    bool setData(const QModelIndex &index, const QVariant &value, int role);
    void addUrlCheck(const QString &url, bool checked = false, bool ok = false);
    void urlChecked(int row, bool ok);
    void urlChecked(const QString &url, bool ok);
    void removeUrlCheck(int row);

signals:
    void countChanged(int count);

private:
    QList<UrlCheck> m_list;
#if QT_VERSION >= 0x040600
    QHash<int, QByteArray> m_roleNames;
#endif
};

#endif // URLCHECKER_H
