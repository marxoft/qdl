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

#include "solvemedia.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace QtJson;

SolveMedia::SolveMedia(QObject *parent) :
    RecaptchaPlugin(parent)
{
}

void SolveMedia::getCaptcha(const QString &key) {
    this->setKey(key);
    QUrl url("http://api.solvemedia.com/papi/_challenge.js?k=" + key);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SolveMedia::onCaptchaDownloaded() {
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
        bool ok = false;
        QVariantMap map = Json::parse(response, ok).toMap();

        if (ok) {
            QString challenge = map.value("ACChallengeResult").toMap().value("chid").toString();

            if (challenge.isEmpty()) {
                emit error(CaptchaNotFound);
            }
            else {
                this->downloadCaptchaImage(challenge);
            }
        }
        else {
            emit error(CaptchaNotFound);
        }
    }

    reply->deleteLater();
}

void SolveMedia::downloadCaptchaImage(const QString &challenge) {
    this->setChallenge(challenge);
    QUrl url("http://api.solvemedia.com/papi/media?c=" + challenge);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaImageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void SolveMedia::onCaptchaImageDownloaded() {
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

void SolveMedia::reportError(int errorCode) {
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

bool SolveMedia::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(solvemedia, SolveMedia)
