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

#include "deathbycaptcha.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>

using namespace QtJson;

DeathByCaptcha::DeathByCaptcha(QObject *parent) :
    DecaptchaPlugin(parent),
    m_formPost(0)
{
}

void DeathByCaptcha::getCaptchaResponse(const QByteArray &data) {

    if (!m_formPost) {
        m_formPost = new FormPostPlugin(this);
    }

    m_formPost->addField("username", username());
    m_formPost->addField("password", password());
    m_formPost->addFile("captchafile", data, "captcha.jpg", "image/jpeg");
    QNetworkReply *reply = m_formPost->postData("http://api.dbcapi.me/api/captcha");
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void DeathByCaptcha::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 303) {
        m_statusUrl = reply->header(QNetworkRequest::LocationHeader).toUrl();

        if (m_statusUrl.isValid()) {
            QTimer::singleShot(10000, this, SLOT(checkCaptchaStatus()));
        }
        else {
            emit error(UnknownError);
        }
    }
    else {
        this->reportError(statusCode);
    }

    reply->deleteLater();
}

void DeathByCaptcha::checkCaptchaStatus() {
    QNetworkRequest request(m_statusUrl);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaStatusResponse()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void DeathByCaptcha::checkCaptchaStatusResponse() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode != 200) {
        this->reportError(statusCode);
    }
    else {

        QString response(reply->readAll());
        QVariantMap map = Json::parse(response).toMap();
        m_captchaId = map.value("captcha").toString();
        bool success = map.value("is_correct").toBool();
        QString text = map.value("text").toString();

        if (!success) {
            emit error(CaptchaUnsolved);
        }
        else if (!text.isEmpty()) {
            emit captchaResponseReady(text);
        }
        else {
            QTimer::singleShot(5000, this, SLOT(checkCaptchaStatus()));
        }
    }

    reply->deleteLater();
}

void DeathByCaptcha::reportIncorrectCaptchaResponse(const QString &id) {
    QUrl url(QString("http://api.dbcapi.me/api/captcha/%1/report").arg(id));
    QString postData = QString("username=%1&password=%2").arg(username()).arg(password());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, postData.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaReported()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void DeathByCaptcha::onCaptchaReported() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode != 200) {
        this->reportError(statusCode);
    }

    reply->deleteLater();
}

void DeathByCaptcha::reportError(int errorCode) {
    switch (errorCode) {
    case 404:
        emit error(CaptchaNotFound);
        break;
    case 503:
        emit error(ServiceUnavailable);
        break;
    case 500:
        emit error(InternalError);
        break;
    case 403:
        emit error(Unauthorised);
        break;
    default:
        emit error(UnknownError);
    }
}

bool DeathByCaptcha::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(deathbycaptcha, DeathByCaptcha)
#endif
