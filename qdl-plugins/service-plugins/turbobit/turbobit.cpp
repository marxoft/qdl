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

#include "turbobit.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

TurboBit::TurboBit(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp TurboBit::urlPattern() const {
    return QRegExp("http://(www.|)turbobit.net/\\w+", Qt::CaseInsensitive);
}

bool TurboBit::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void TurboBit::login(const QString &username, const QString &password) {
    QString data = QString("user[login]=%1&user[pass]=%2&user[submit]=Login").arg(username).arg(password);
    QUrl url("http://turbobit.net/user/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TurboBit::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    emit loggedIn((statusCode == 200) || (statusCode == 302));

    reply->deleteLater();
}

void TurboBit::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TurboBit::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString response(reply->readAll());

    if (response.contains(QRegExp("File not found|service is currently unavailable in your country"))) {
        emit urlChecked(false);
    }
    else {
        QString fileName = response.section(QRegExp("keywords\" content=\"\\s+"), 1, 1).section(',', 0, 0);

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void TurboBit::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TurboBit::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://turbobit.net/download/redirect/\\w+/\\w+/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(redirect));
        emit downloadRequestReady(request);
    }
    else if (!redirect.isEmpty()) {
        this->getDownloadRequest(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        m_fileId = response.section("href=\"/download/free/", 1, 1).section('"', 0, 0);

        if (m_fileId.isEmpty()) {
            emit error(UnknownError);
        }
        else {
            this->getCaptcha();
        }
    }

    reply->deleteLater();
}

void TurboBit::getCaptcha() {
    QUrl url("http://turbobit.net/download/free/" + m_fileId);
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkCaptcha()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TurboBit::checkCaptcha() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://turbobit.net/download/redirect/\\w+/\\w+/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(redirect));
        emit downloadRequestReady(request);
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QNetworkRequest request;
            request.setUrl(QUrl(re.cap()));
            emit downloadRequestReady(request);
        }
        else if (response.contains("limit of connections is reached")) {
            int secs = response.section("id='timeout'>", 1, 1).section('<', 0, 0).toInt();

            if (secs > 0) {
                this->startWait(secs * 1000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
            }
            else {
                emit error(UnknownError);
            }
        }
        else {
            m_captchaLink = response.section("Captcha\" src=\"", 1, 1).section('"', 0, 0);
            m_captchaType = response.section("' name = 'captcha_type", 0, 0).section('\'', -1);
            m_captchaSubtype = response.section("' name = 'captcha_subtype", 0, 0).section('\'', -1);

            if ((m_captchaLink.isEmpty()) || (m_captchaType.isEmpty())) {
                emit error(UrlError);
            }
            else {
                emit statusChanged(CaptchaRequired);
            }
        }
    }

    reply->deleteLater();
}

void TurboBit::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void TurboBit::submitCaptchaResponse(const QString &challenge, const QString &response) {
    Q_UNUSED(challenge)

    QUrl url(QString("http://turbobit.net/download/free/%1#").arg(m_fileId));
    QString data = QString("captcha_response=%1&captcha_type=%2&captcha_subtype=%3").arg(response).arg(m_captchaType).arg(m_captchaSubtype);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", "http://turbobit.net/download/free/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TurboBit::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    if (response.contains("captcha-error")) {
        emit error(CaptchaError);
    }
    else if (response.contains("limit of connections is reached")) {
        int secs = response.section("id='timeout'>", 1, 1).section('<', 0, 0).toInt();

        if (secs > 0) {
            this->startWait(secs * 1000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            emit error(UnknownError);
        }
    }
    else {
        int secs = response.section("maxLimit : ", 1, 1).section(' ', 0, 0).toInt();

        if (secs > 0) {
            this->startWait(secs * 1000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void TurboBit::getDownloadLink() {
    QUrl url("http://turbobit.net/download/getLinkTimeout/" + m_fileId);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TurboBit::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QUrl url("http://turbobit.net" + response.section("href\")==\"", 1, 1).section('"', 0, 0));

    if (url.isValid()) {
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void TurboBit::startWait(int msecs) {
    if (msecs > 90000) {
        emit statusChanged(LongWait);
    }
    else {
        emit statusChanged(ShortWait);
    }

    emit waiting(msecs);
    m_waitTime = msecs;
    m_waitTimer->start(1000);
}

void TurboBit::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void TurboBit::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool TurboBit::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(turbobit, TurboBit)
