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

#include "mediafire.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

using namespace QtJson;

MediaFire::MediaFire(QObject *parent) :
    ServicePlugin(parent),
    m_captchaService("SolveMedia")
{
}

QRegExp MediaFire::urlPattern() const {
    return QRegExp("http(s|)://(www.|)mediafire.com/(\\?|)\\w+", Qt::CaseInsensitive);
}

bool MediaFire::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void MediaFire::login(const QString &username, const QString &password) {
    QString data = QString("login_email=%1&login_pass=%2").arg(username).arg(password);
    QUrl url("http://www.mediafire.com/dynamic/login.php");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MediaFire::checkLogin() {
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
        emit loggedIn(true);
        break;
    default:
        emit loggedIn(false);
        break;
    }

    reply->deleteLater();
}

void MediaFire::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MediaFire::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://download\\d+.mediafire.com/[^'\"]+");

    if (!redirect.isEmpty()) {
        if (re.indexIn(redirect) == 0) {
            emit urlChecked(true, reply->request().url(), this->serviceName(), redirect.section('/', -1));
        }
        else if (redirect.contains("error.php")) {
            emit urlChecked(false);
        }
        else if (redirect.startsWith('/')) {
            QUrl url = reply->request().url();
            this->checkUrl(QUrl(QString("%1://%2%3").arg(url.scheme()).arg(url.host()).arg(redirect)));
        }
        else {
            this->checkUrl(QUrl(redirect));
        }
    }
    else {
        QString response(reply->readAll());

        if (response.contains("thisfolder_root")) {
            this->getFolderFileLinks(reply->request().url().toString().section('?', -1));
        }
        else {
            QString fileName = response.section("og:title\" content=\"", 1, 1).section('"', 0, 0);

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

void MediaFire::getFolderFileLinks(const QString &folderId) {
    QUrl url("http://www.mediafire.com/api/folder/get_content.php");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("content_type", "files");
    query.addQueryItem("order_by", "name");
    query.addQueryItem("order_direction", "asc");
    query.addQueryItem("chunk", "1");
    query.addQueryItem("version", "2");
    query.addQueryItem("folder_key", folderId);
    query.addQueryItem("response_format", "json");
    url.setQuery(query);
#else
    url.addQueryItem("content_type", "files");
    url.addQueryItem("order_by", "name");
    url.addQueryItem("order_direction", "asc");
    url.addQueryItem("chunk", "1");
    url.addQueryItem("version", "2");
    url.addQueryItem("folder_key", folderId);
    url.addQueryItem("response_format", "json");
#endif
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkFolderFileLinks()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MediaFire::checkFolderFileLinks() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString response(reply->readAll());
    QVariantMap folder = Json::parse(response).toMap();
    QVariantList files = folder.value("response").toMap().value("folder_content").toMap().value("files").toList();

    if (files.isEmpty()) {
        emit urlChecked(false);
    }
    else {
        while (!files.isEmpty()) {
            QVariantMap file = files.takeFirst().toMap();
            QString id = file.value("quickkey").toString();
            QString fileName = file.value("filename").toString();

            if ((!id.isEmpty()) && (!fileName.isEmpty())) {
                QUrl url("http://www.mediafire.com/?" + id);
                emit urlChecked(true, url, this->serviceName(), fileName, files.isEmpty());
            }
            else if (files.isEmpty()) {
                emit urlChecked(false);
            }
        }
    }

    reply->deleteLater();
}

void MediaFire::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_url = webUrl;
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MediaFire::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://download\\d+.mediafire.com/[^'\"]+");
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
            m_captchaKey = response.section("api.solvemedia.com/papi/challenge.script?k=", 1, 1).section('"', 0, 0);
            m_captchaUrl.setUrl("http://www.mediafire.com" + response.section("form_captcha\" action=\"", 1, 1).section('"', 0, 0));

            if (m_captchaKey.isEmpty()) {
                m_captchaService = "Google";
                m_captchaKey = response.section("www.google.com/recaptcha/api/noscript?k=", 1, 1).section('"', 0, 0);

                if (m_captchaKey.isEmpty()) {
                    emit error(UnknownError);
                }
                else {
                    emit statusChanged(CaptchaRequired);
                }
            }
            else {
                emit statusChanged(CaptchaRequired);
            }
        }
    }

    reply->deleteLater();
}

void MediaFire::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data;
    
    if (this->recaptchaServiceName() == "Google") {
        data = QString("recaptcha_challenge_field=%1&recaptcha_response_field=%2").arg(challenge).arg(response);
    }
    else {
        data = QString("adcopy_challenge=%1&adcopy_response=%2").arg(challenge).arg(response);
    }

    QNetworkRequest request(m_captchaUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MediaFire::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://download\\d+.mediafire.com/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else if (!redirect.isEmpty()) {
        QNetworkRequest request;
        request.setUrl(QUrl(redirect));
        QNetworkReply *redirectReply = this->networkAccessManager()->get(request);
	    this->connect(redirectReply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
        this->connect(this, SIGNAL(currentOperationCancelled()), redirectReply, SLOT(deleteLater()));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QNetworkRequest request;
            request.setUrl(QUrl(re.cap()));
            emit downloadRequestReady(request);
        }
        else if (response.contains("Your entry was incorrect")) {
            emit error(CaptchaError);
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

bool MediaFire::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(mediafire, MediaFire)
#endif
