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

#include "uploaded.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>
#include <QStringList>

using namespace QtJson;

Uploaded::Uploaded(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Uploaded::urlPattern() const {
    return QRegExp("(http(s|)://(www.|)uploaded.(net|to)/f(ile|)/|http://ul.to/)\\w+", Qt::CaseInsensitive);
}

bool Uploaded::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Uploaded::login(const QString &username, const QString &password) {
    QString data = QString("id=%1&pw=%2").arg(username).arg(password);
    QUrl url("http://uploaded.net/io/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploaded::checkLogin() {
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

void Uploaded::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploaded::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://([-\\w]+|)stor\\d+.uploaded.net/dl/[-\\w]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        if (redirect == "http://uploaded.net/404") {
            emit urlChecked(false);
        }
        else {
            this->checkUrl(QUrl(redirect));
        }
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("id=\"filename\">", 1, 1).section("<", 0, 0).trimmed().replace("&hellip;", ".");

        if (fileName.isEmpty()) {
            // Check if it's a folder

            if (response.contains("id=\"fileList\">")) {
                QString table = response.section("id=\"fileList\">", 1, 1).section("</table>", 0, 0);
                QStringList files = table.split("id=\"", QString::SkipEmptyParts);

                if (files.isEmpty()) {
                    emit urlChecked(false);
                }
                else {
                    while (!files.isEmpty()) {
                        QString file = files.takeFirst();
                        QString id = file.left(file.indexOf('"'));
                        QString fileName = file.section("this))\">", 1, 1).section('<', 0, 0);

                        if ((!id.isEmpty()) && (!fileName.isEmpty())) {
                            emit urlChecked(true, QUrl("http://uploaded.net/file/" + id), this->serviceName(), fileName, files.isEmpty());
                        }
                    }
                }
            }
            else {
                emit urlChecked(false);
            }
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void Uploaded::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_fileId = webUrl.toString().section(QRegExp("/file/|/ul.to/"), -1);
    QUrl url("http://uploaded.net/file/" + m_fileId);
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploaded::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://([-\\w]+|)stor\\d+.uploaded.net/dl/[-\\w]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (redirect == "http://uploaded.net/404") {
        emit error(NotFound);
    }
    else if (re.indexIn(redirect) == 0) {
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
            int secs = response.section("Current waiting period: <span>", 1, 1).section('<', 0, 0).toInt();
            m_captchaKey = "6Lcqz78SAAAAAPgsTYF3UlGf2QFQCNuPMenuyHF3"; //response.section("Recaptcha.create('", 1, 1).section('\'', 0, 0);

            if ((secs <= 0) || (m_captchaKey.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                this->startWait(secs * 1000);

                if (secs > 30) {
                    this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
                }
                else {
                    this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
                }
            }
        }
    }

    reply->deleteLater();
}

void Uploaded::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void Uploaded::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("recaptcha_challenge_field=%1&recaptcha_response_field=%2").arg(challenge).arg(response);
    QUrl url("http://uploaded.net/io/ticket/captcha/" + m_fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "text/javascript, text/html, application/xml, text/xml, */*");
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("X-Prototype-Version", "1.6.1");
    request.setRawHeader("Host", "uploaded.net");
    request.setRawHeader("Origin", "http://uploaded.net");
    request.setRawHeader("Referer", "http://uploaded.net/file/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploaded::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QUrl url(response.section("url:'", 1, 1).section('\'', 0, 0));

    if (url.isValid()) {
        QNetworkRequest request(url);
        request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        request.setRawHeader("Host", url.host().toUtf8());
        request.setRawHeader("Referer", "http://uploaded.net/file/" + m_fileId.toUtf8());
        request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.4 (KHTML, like Gecko) Ubuntu/12.10 Chromium/22.0.1229.94 Chrome/22.0.1229.94 Safari/537.4");
        emit downloadRequestReady(request);
    }
    else {
        QVariantMap map = Json::parse(response).toMap();
        QString errorString = map.value("err").toString();

        if (errorString == "captcha") {
            emit error(CaptchaError);
        }
        else if ((errorString == "limit-dl") || (errorString.contains(QRegExp("max|Download-Slots|Free-Downloads", Qt::CaseInsensitive)))) {
            this->startWait(600000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void Uploaded::startWait(int msecs) {
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

void Uploaded::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Uploaded::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Uploaded::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(uploaded, Uploaded)
#endif
