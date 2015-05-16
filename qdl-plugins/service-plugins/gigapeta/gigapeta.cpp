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

#include "gigapeta.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QTime>
#include <QRegExp>

GigaPeta::GigaPeta(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp GigaPeta::urlPattern() const {
    return QRegExp("http(s|)://(www.|)gigapeta.com/dl/\\w+", Qt::CaseInsensitive);
}

bool GigaPeta::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void GigaPeta::login(const QString &username, const QString &password) {
    QString data = QString("auth_login=%1&auth_passwd=%2").arg(username).arg(password);
    QUrl url("http://gigapeta.com");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void GigaPeta::checkLogin() {
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

void GigaPeta::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void GigaPeta::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+.gigapeta.com/download\\?[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("alt=\"file\" />-->", 1, 1).section('<', 0, 0).trimmed();

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void GigaPeta::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void GigaPeta::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.gigapeta.com/download\\?[^'\"]+");
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
        m_url = reply->request().url();

        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QNetworkRequest request;
            request.setUrl(QUrl(re.cap()));
            emit downloadRequestReady(request);
        }
        else {
            int secs = response.section("time left <b>", 1, 1).section('<', 0, 0).trimmed().toInt();

            if (secs > 0) {
                this->startWait(secs * 1000);

                if (secs > 40) {
                    this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
                }
                else {
                    this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
                }
            }
            else if (response.contains("big_error\">404")) {
                emit error(NotFound);
            }
            else if (response.contains("ability to download next file after")) {
                QString waitTime = response.section("ability to download next file after <b>", 1, 1).section('<', 0, 0);
                QRegExp re("(\\d+)");
                int hours = 0;
                int mins = 0;

                if (re.indexIn(waitTime) != -1) {
                    if (waitTime.contains("hr.")) {
                        hours = re.cap(1).toInt();

                        if (re.indexIn(waitTime, re.matchedLength())) {
                            mins = re.cap(1).toInt();
                        }
                    }
                    else {
                        mins = re.cap(1).toInt();
                    }
                }

                QTime time(hours, mins);

                if (time.isValid()) {
                    this->startWait(QTime().msecsTo(time));
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
    }

    reply->deleteLater();
}

void GigaPeta::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void GigaPeta::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("captcha_key=%1&captcha=%2&download=Download").arg(challenge).arg(response);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void GigaPeta::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.gigapeta.com/download\\?[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QNetworkRequest request;
            request.setUrl(QUrl(re.cap()));
            emit downloadRequestReady(request);
        }
        else if (response.contains("coincide with the picture")) {
            int secs = response.section("time left <b>", 1, 1).section('<', 0, 0).trimmed().toInt();

            if (secs > 0) {
                this->startWait(secs * 1000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
            }
            else {
                emit error(CaptchaError);
            }
        }
        else if (response.contains("ability to download next file after")) {
            QString waitTime = response.section("ability to download next file after <b>", 1, 1).section('<', 0, 0);
            QRegExp re("(\\d+)");
            int hours = 0;
            int mins = 0;

            if (re.indexIn(waitTime) != -1) {
                if (waitTime.contains("hr.")) {
                    hours = re.cap(1).toInt();

                    if (re.indexIn(waitTime, re.matchedLength())) {
                        mins = re.cap(1).toInt();
                    }
                }
                else {
                    mins = re.cap(1).toInt();
                }
            }

            QTime time(hours, mins);

            if (time.isValid()) {
                this->startWait(QTime().msecsTo(time));
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

    reply->deleteLater();
}

void GigaPeta::startWait(int msecs) {
    if (msecs > 40000) {
        emit statusChanged(LongWait);
    }
    else {
        emit statusChanged(ShortWait);
    }

    emit waiting(msecs);
    m_waitTime = msecs;
    m_waitTimer->start(1000);
}

void GigaPeta::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void GigaPeta::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool GigaPeta::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(gigapeta, GigaPeta)
#endif
