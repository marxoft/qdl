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

#include "billionuploads.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>
#include <algorithm>

BillionUploads::BillionUploads(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp BillionUploads::urlPattern() const {
    return QRegExp("http(s|)://(www.|)billionuploads.com/\\w+", Qt::CaseInsensitive);
}

bool BillionUploads::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void BillionUploads::login(const QString &username, const QString &password) {
    QString data = QString("op=login&login=%1&password=%2").arg(username).arg(password);
    QUrl url("http://billionuploads.com/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BillionUploads::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (statusCode) {
    case 302:
    case 200:
    case 201:
        m_connections = 0;
        emit loggedIn(true);
        break;
    default:
        m_connections = 1;
        emit loggedIn(false);
        break;
    }

    reply->deleteLater();
}

void BillionUploads::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BillionUploads::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\d+.\\d+.\\d+.\\d+:\\d+/d/\\w+/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("dofir\" title=\"", 1, 1).section("\">", 0, 0).trimmed();

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void BillionUploads::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BillionUploads::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\d+.\\d+.\\d+.\\d+:\\d+/d/\\w+/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else if (!redirect.isEmpty()) {
        this->getDownloadRequest(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QNetworkRequest request;
            request.setUrl(QUrl(re.cap()));
            emit downloadRequestReady(request);
        }
        else if (response.contains("File Not Found")) {
            emit error(NotFound);
        }
        else {
            QString id = response.section("id\" value=\"", 1, 1).section('"', 0, 0);
            QString rand = response.section("rand\" value=\"", 1, 1).section('"', 0, 0);
            QString hidden1 = QByteArray::fromPercentEncoding(response.section("innerHTML=decodeURIComponent(\"", 1, 1)
                                                                      .section("\")", 0, 0));
                                                                      
            QString hidden1Name = hidden1.section("name=\"", 1, 1).section('"', 0, 0);
            QString hidden1Value = hidden1.section("value=\"", 1, 1).section('"', 0, 0);
            QString hidden2Name = response.section("attr('type','hidden').attr('name','", 1, 1).section("')", 0, 0);
            QString hidden2Value = response.section("visibility: hidden\">", 1, 1).section("</textarea>", 0, 0);

            if ((id.isEmpty()) || (rand.isEmpty()) ||
                (hidden1Name.isEmpty()) || (hidden1Value.isEmpty()) ||
                (hidden2Name.isEmpty()) || (hidden2Value.isEmpty())) {
                
                QString errorString = response.section("<div class=\"err\">", 1, 1).section('<', 0, 0);

                if (!errorString.isEmpty()) {
                    if (errorString.startsWith("You have to wait")) {
                        int mins = errorString.section(" minutes", 0, 0).section(' ', -1).toInt();
                        int secs = errorString.section(" seconds", 0, 0).section(' ', -1).toInt();
                        this->startWait((mins * 60000) + (secs * 1000));
                        this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
                    }
                    else {
                        emit error(UnknownError);
                    }
                }
                else {
                    emit error(UnknownError);
                }
            }
            else {
                QString data = QString("op=download2&id=%1&rand=%2&%3=%4&%5=%6&down_direct=1")
                                      .arg(id).arg(rand).arg(hidden1Name).arg(hidden1Value)
                                      .arg(hidden2Name).arg(hidden2Value);
                                      
                this->getDownloadLink(reply->url(), data);
            }
        }
    }

    reply->deleteLater();
}

void BillionUploads::getDownloadLink(const QUrl &url, const QString &data) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BillionUploads::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\d+.\\d+.\\d+.\\d+:\\d+/d/\\w+/[^'\"]+");
    QString response(reply->readAll());

    if (re.indexIn(response) >= 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else {
        QByteArray fastUrl = response.section("var fasturl = '", 1, 1).section("';", 0, 0).toUtf8();
        
        if (fastUrl.isEmpty()) {
            emit error(UnknownError);
        }
        else {
            std::reverse(fastUrl.constBegin(), fastUrl.constEnd());
            emit downloadRequestReady(QNetworkRequest(QUrl::fromEncoded(fastUrl)));
        }
    }

    reply->deleteLater();
}

void BillionUploads::startWait(int msecs) {
    if (msecs > 30000) {
        emit statusChanged(LongWait);
    }
    else {
        emit statusChanged(ShortWait);
    }

    emit waiting(msecs);
    m_waitTime = msecs;
    m_waitTimer->start(1000);
}

void BillionUploads::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void BillionUploads::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool BillionUploads::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(billionuploads, BillionUploads)
#endif
