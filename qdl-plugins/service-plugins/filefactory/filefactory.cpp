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

#include "filefactory.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

using namespace QtJson;

FileFactory::FileFactory(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp FileFactory::urlPattern() const {
    return QRegExp("http(s|)://(www\\.|)filefactory.com/(file|stream)/\\w+", Qt::CaseInsensitive);
}

bool FileFactory::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void FileFactory::login(const QString &username, const QString &password) {
    QString data = QString("email=%1&password=%2").arg(username).arg(password);
    QUrl url("http://www.filefactory.com/member/login.php");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileFactory::checkLogin() {
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

void FileFactory::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileFactory::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w\\d+\\.filefactory.com/get/\\w/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        if (redirect.startsWith("/")) {
            redirect.prepend("http://www.filefactory.com");
        }
        
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains(QRegExp("file is no longer available|file has been deleted"))) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("file_name", 1, 1).section("<h2>", 1, 1).section('<', 0, 0);

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

void FileFactory::getDownloadRequest(const QUrl &webUrl) {
    if (m_url.isValid()) {
        emit downloadRequestReady(QNetworkRequest(m_url));
        m_url.clear();
    }
    else {
        emit statusChanged(Connecting);
        QNetworkRequest request(webUrl);
        request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
        QNetworkReply *reply = this->networkAccessManager()->get(request);
        this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
        this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    }
}

void FileFactory::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w\\d+\\.filefactory.com/get/\\w/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else if (!redirect.isEmpty()) {
        if (redirect.startsWith("/")) {
            redirect.prepend("http://www.filefactory.com");
        }
        
        this->getDownloadRequest(QUrl(redirect));
    }
    else {
        QString response(reply->readAll().simplified());

        if (re.indexIn(response) >= 0) {
            m_url = QUrl(re.cap());
            int secs = response.section("data-delay=\"", 1, 1).section('"', 0, 0).toInt();

            if (secs > 0) {
                this->startWait(secs * 1000);
                
            }
            else {
                this->startWait(60000);
            }

            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else if (response.contains(QRegExp("file is no longer available|file has been deleted"))) {
            emit error(NotFound);
        }
        else {
            m_check = response.section("check: '", 1, 1).section('\'', 0, 0);
            m_captchaKey = response.section("Recaptcha.create( \"", 1, 1).section('"', 0, 0);

            if ((m_check.isEmpty()) || (m_captchaKey.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                emit statusChanged(CaptchaRequired);
            }
        }
    }

    reply->deleteLater();
}

void FileFactory::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("recaptcha_challenge_field=%1&recaptcha_response_field=%2&check=%3").arg(challenge).arg(response).arg(m_check);
    QUrl url("http://www.filefactory.com/file/checkCaptcha.php");
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileFactory::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap map = Json::parse(response).toMap();

    if (map.value("status") == "ok") {
        QString path = map.value("path").toString();

        if (!path.isEmpty()) {
            this->getDownloadPage(QUrl("http://www.filefactory.com" + path));
        }
        else {
            emit error(UnknownError);
        }
    }
    else if (map.value("message").toString().startsWith("Entered code")) {
        emit error(CaptchaError);
    }
    else {
        emit error(UrlError);
    }

    reply->deleteLater();
}

void FileFactory::getDownloadPage(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileFactory::checkDownloadPage() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QRegExp re("http://\\w\\d+\\.filefactory.com/get/\\w/[^'\"]+");

    if (re.indexIn(response) >= 0) {
        m_url = QUrl(re.cap());

        int secs = response.section("data-delay=\"", 1, 1).section('"', 0, 0).toInt();

        if (secs > 0) {
            this->startWait(secs * 1000);
            
        }
        else {
            this->startWait(60000);
        }

        this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void FileFactory::startWait(int msecs) {
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

void FileFactory::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void FileFactory::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool FileFactory::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(filefactory, FileFactory)
#endif
