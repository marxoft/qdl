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

#include "megashares.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QTimer>
#include <QDateTime>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

MegaShares::MegaShares(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp MegaShares::urlPattern() const {
    return QRegExp("http://d01.megashares.com/dl/\\w+", Qt::CaseInsensitive);
}

bool MegaShares::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void MegaShares::login(const QString &username, const QString &password) {
    QString data = QString("mymslogin_name=%1&mymspassword=%2").arg(username).arg(password);
    QUrl url("http://d01.megashares.com/myms_login.php");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MegaShares::checkLogin() {
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

void MegaShares::checkUrl(const QUrl &webUrl) {
    QString urlString = webUrl.toString();
    m_fileName = urlString.section('/', -1);
    QString id = urlString.section("/dl/", 1, 1).section('/', 0, 0);
    QUrl url("http://d01.megashares.com/index.php?d01=" + id);
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MegaShares::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://webprod\\d+\\.megashares.com/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("letter-spacing: -1px\" title=\"", 1, 1).section('"', 0, 0);

        if (fileName.isEmpty()) {
            QString errorString = response.section("class=\"red\">", 1, 1).section('<', 0, 0);

            if (errorString.startsWith("All download slots")) {
                emit urlChecked(true, reply->request().url(), this->serviceName(), m_fileName);
            }
            else {
                emit error(UnknownError);
            }
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void MegaShares::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_url = webUrl;
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MegaShares::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://webprod\\d+\\.megashares.com/[^'\"]+");
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
            m_downloadUrl.setUrl(re.cap());

            if (response.contains("Your Passport needs to be reactivated")) {
                m_captchaKey = response.section("random_num\" value=\"", 1, 1).section('"', 0, 0);
                m_passportKey = response.section("passport_num\" value=\"", 1, 1).section('"', 0, 0);

                if ((m_captchaKey.isEmpty()) || (m_passportKey.isEmpty())) {
                    emit error(UnknownError);
                }
                else {
                    emit statusChanged(CaptchaRequired);
                }
            }
            else {
                emit downloadRequestReady(QNetworkRequest(m_downloadUrl));
            }
        }
        else {
            QString errorString = response.section("class=\"red\">", 1, 1).section('<', 0, 0);

            if (errorString.startsWith("Invalid link")) {
                emit error(NotFound);
            }
            else if (errorString.startsWith("All download slots")) {
                this->startWait(600000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
            }
            else {
                emit error(UnknownError);
            }
        }
    }

    reply->deleteLater();
}

void MegaShares::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QUrl url(m_url);
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("rs", "check_passport_renewal");
    query.addQueryItem("rsargs[]", challenge);
    query.addQueryItem("rsargs[]", response);
    query.addQueryItem("rsargs[]", m_passportKey);
    query.addQueryItem("rsargs[]", "replace_sec_pprenewal");
    query.addQueryItem("rsrnd", QString::number(QDateTime::currentMSecsSinceEpoch()));
    url.setQuery(query);
#else
    url.addQueryItem("rs", "check_passport_renewal");
    url.addQueryItem("rsargs[]", challenge);
    url.addQueryItem("rsargs[]", response);
    url.addQueryItem("rsargs[]", m_passportKey);
    url.addQueryItem("rsargs[]", "replace_sec_pprenewal");
    url.addQueryItem("rsrnd", QString::number(qint64(QDateTime::currentDateTime().toTime_t()) * 1000));
#endif
    QNetworkRequest request(url);
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MegaShares::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    if (response.contains("Invalid renewal code")) {
        m_captchaKey = response.section("random_num\" value=\"", 1, 1).section('"', 0, 0);
        m_passportKey = response.section("passport_num\" value=\"", 1, 1).section('"', 0, 0);

        if ((m_captchaKey.isEmpty()) || (m_passportKey.isEmpty())) {
            emit error(UnknownError);
        }
        else {
            emit error(CaptchaError);
        }
    }
    else {
        emit downloadRequestReady(QNetworkRequest(m_downloadUrl));
    }

    reply->deleteLater();
}

void MegaShares::startWait(int msecs) {
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

void MegaShares::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void MegaShares::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool MegaShares::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(megashares, MegaShares)
