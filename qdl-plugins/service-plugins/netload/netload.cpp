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

#include "netload.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

Netload::Netload(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Netload::urlPattern() const {
    return QRegExp("http(s|)://(www.|)netload.in/\\w+", Qt::CaseInsensitive);
}

bool Netload::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Netload::login(const QString &username, const QString &password) {
    QString data = QString("txtuser=%1&txtpass=%2&txtcheck=login&txtlogin=").arg(username).arg(password);
    QUrl url("http://netload.in/index.php");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Netload::checkLogin() {
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

void Netload::checkUrl(const QUrl &webUrl) {
    QString id = webUrl.toString().section("netload.in/", -1).section(QRegExp("/|\\."), 0, 0);
    QUrl url(QString("http://netload.in/%1.htm").arg(id));
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Netload::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QRegExp re("http://\\d+.\\d+.\\d+.\\d+/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        emit urlChecked(true, reply->request().url(), this->serviceName());
    }
    else {
        QString response(reply->readAll());

        if (response.contains("This file is only for Premium Users")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section(QRegExp("dl_first_filename\">\\s+"), 1, 1).section('<', 0, 0);

            if (fileName.isEmpty()) {
                emit urlChecked(false);
            }
            else {
                emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
            }
        }
    }

    reply->deleteLater();
}

void Netload::getDownloadRequest(const QUrl &webUrl) {
    if (!m_downloadUrl.isEmpty()) {
        emit downloadRequestReady(QNetworkRequest(m_downloadUrl));
        m_downloadUrl.clear();
    }
    else {
        emit statusChanged(Connecting);
        QString id = webUrl.toString().section("netload.in/", -1).section(QRegExp("/|\\."), 0, 0);
        QUrl url(QString("http://netload.in/%1.htm").arg(id));
        QNetworkRequest request(url);
        request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
        QNetworkReply *reply = this->networkAccessManager()->get(request);
        this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
        this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    }
}

void Netload::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\d+\\.\\d+\\.\\d+\\.\\d+/[^'\"]+");
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
        else {
            m_waitUrl = QUrl(QString("http://netload.in/" + response.section("Free_dl\"><a href=\"", 1, 1).section('"', 0, 0)).remove("amp;"));
#if QT_VERSION >= 0x050000
            QUrlQuery query(m_waitUrl);
            m_fileId = query.queryItemValue("file_id");
            m_id = query.queryItemValue("id");
#else
            m_fileId = m_waitUrl.queryItemValue("file_id");
            m_id = m_waitUrl.queryItemValue("id");
#endif
            if ((m_fileId.isEmpty()) || (m_id.isEmpty()) || (!m_waitUrl.isValid())) {
                emit error(UrlError);
            }
            else {
                this->getWaitTime();
            }
        }
    }

    reply->deleteLater();
}

void Netload::getWaitTime() {
    QNetworkRequest request(m_waitUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Netload::checkWaitTime() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    int secs = response.section("text/javascript\">countdown(", 1, 1).section(',', 0, 0).toInt();
    m_captchaKey = response.section("share/includes/captcha.php?t=", 1, 1).section('"', 0, 0);

    if (secs > 0) {
        this->startWait(secs * 10);

        if (m_captchaKey.isEmpty()) {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
        }
        else {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
        }
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void Netload::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void Netload::submitCaptchaResponse(const QString &challenge, const QString &response) {
    Q_UNUSED(challenge);

    QUrl url("http://netload.in/index.php?id=" + m_id);
    QString data = QString("file_id=%1&captcha_check=%2&start=").arg(m_fileId).arg(response);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_waitUrl.toString().toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Netload::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\d+\\.\\d+\\.\\d+\\.\\d+/[^'\"]+");
    QString response(reply->readAll());

    if (re.indexIn(response) >= 0) {
        m_downloadUrl = QUrl(re.cap());
        int secs = response.section("text/javascript\">countdown(", 1, 1).section(',', 0, 0).toInt();

        if (secs > 0) {
            this->startWait(secs * 10);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            emit error(UnknownError);
        }
    }
    else if (response.contains("class=\"InPage_Error")) {
        emit error(CaptchaError);
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void Netload::getDownloadLink() {
    QUrl url("http://netload.in/index.php?id=" + m_id);
    QString data = QString("file_id=%1").arg(m_fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_waitUrl.toString().toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
}

void Netload::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\d+.\\d+.\\d+.\\d+/[^'\"]+");
    QString response(reply->readAll());

    if (re.indexIn(response) >= 0) {
        m_downloadUrl = QUrl(re.cap());
        int secs = response.section("text/javascript\">countdown(", 1, 1).section(')', 0, 0).toInt();

        if (secs > 0) {
            this->startWait(secs * 10);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
    }
    else {
        emit error(UrlError);
    }

    reply->deleteLater();
}

void Netload::startWait(int msecs) {
    if (msecs > 60000) {
        emit statusChanged(LongWait);
    }
    else {
        emit statusChanged(ShortWait);
    }

    emit waiting(msecs);
    m_waitTime = msecs;
    m_waitTimer->start(1000);
}

void Netload::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Netload::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Netload::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(netload, Netload)
#endif
