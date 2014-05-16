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

#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H

#include <QObject>
#include <QList>
#include <QHostAddress>

class QHttpServer;
class QHttpRequest;
class QHttpResponse;

class WebInterface : public QObject
{
    Q_OBJECT

public:
    static WebInterface* instance();

    quint16 port() const;

public slots:
    bool start();
    void stop();

    void setPort(quint16 port);

    bool setEnabled(bool enable);

private:
    void loadHosts();
    bool isAllowed(const QHostAddress &address);

private slots:
    void onNewRequest(QHttpRequest *request, QHttpResponse *response);

private:
    WebInterface();
    ~WebInterface();

    static WebInterface *self;

    QHttpServer *m_server;

    quint16 m_port;

    QString m_path;

    QList<QHostAddress> m_hosts;
};

#endif //WEBINTERFACE_H
