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

#include "fileboom.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

FileBoom::FileBoom(QObject *parent) :
    RecaptchaPlugin(parent)
{
}

void FileBoom::getCaptcha(const QString &key) {
    this->setChallenge(key);
    QUrl url("http://fboom.me/file/captcha.html");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("v", key);
    url.setQuery(query);
#else
    url.addQueryItem("v", key);
#endif
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileBoom::onCaptchaDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 200) {
        emit captchaReady(reply->readAll());
    }
    else {
        this->reportError(statusCode);
    }

    reply->deleteLater();
}

void FileBoom::reportError(int errorCode) {
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

bool FileBoom::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(fileboomrecaptcha, FileBoom)
