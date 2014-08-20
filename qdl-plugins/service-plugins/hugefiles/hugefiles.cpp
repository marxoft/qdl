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

#include "hugefiles.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QStringList>

HugeFiles::HugeFiles(QObject *parent) :
    ServicePlugin(parent),
    m_captchaService("SolveMedia"),
    m_connections(1)
{
}

QRegExp HugeFiles::urlPattern() const {
    return QRegExp("http(s|)://(www.|)hugefiles.net/\\w+", Qt::CaseInsensitive);
}

bool HugeFiles::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void HugeFiles::login(const QString &username, const QString &password) {
    QString data = QString("op=login&redirect=&login=%1&password=%2").arg(username).arg(password);
    QUrl url("http://hugefiles.net");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void HugeFiles::checkLogin() {
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

void HugeFiles::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void HugeFiles::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http(s|)://\\d+\\.\\d+\\.\\d+\\.\\d+:\\d+/d/\\w+/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("File Not Found")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("fname\" value=\"", 1, 1).section('"', 0, 0);

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

void HugeFiles::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void HugeFiles::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http(s|)://\\d+\\.\\d+\\.\\d+\\.\\d+:\\d+/d/\\w+/[^'\"]+");
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
        else if (response.contains("file not found", Qt::CaseInsensitive)) {
            emit error(NotFound);
        }
        else {
            m_fileId = response.section("id\" value=\"", 1, 1).section('"', 0, 0);
            m_fileName = response.section("fname\" value=\"", 1, 1).section('"', 0, 0);
            m_rand = response.section("rand\" value=\"", 1, 1).section('"', 0, 0);
            m_captchaType = response.section("ctype\" value=\"", 1, 1).section('"', 0, 0);

            if ((m_fileId.isEmpty()) || (m_fileName.isEmpty()) || (m_captchaType.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                if (response.contains("class=\"captcha_code\"")) {
                    QString codeBlock = response.section("<td align=right>", 1, 1).section("</span></div>", 0, 0);
                    QStringList codeList = codeBlock.split("padding-left:", QString::SkipEmptyParts);

                    if (!codeList.isEmpty()) {
                        codeList.removeFirst();

                        QMap<int, QString> codeMap;

                        int i = 1;

                        while ((!codeList.isEmpty()) && (i < 5)) {
                            QString code = codeList.takeFirst();
                            int key = code.section('p', 0, 0).toInt();
                            QString value = QString::number(code.section(">&#", 1, 1).left(2).toInt() - 48);
                            codeMap[key] = value;
                            i++;
                        }

                        QString captchaCode;

                        foreach (QString val, codeMap.values()) {
                            captchaCode.append(val);
                        }

                        this->submitCaptchaResponse("captcha_code", captchaCode);
                    }
                    else {
                        emit error(UnknownError);
                    }
                }
                else {
                    if (response.contains("api.solvemedia.com")) {
                        m_captchaService = "SolveMedia";
                        m_captchaKey = response.section("api.solvemedia.com/papi/challenge.noscript?k=", 1, 1).section('"', 0, 0);
                    }
                    else if (response.contains("google.com/recaptcha")) {
                        m_captchaService = "Google";
                        m_captchaKey = response.section("google.com/recaptcha/api/challenge?k=", 1, 1).section('"', 0, 0);
                    }

                    if (m_captchaKey.isEmpty()) {
                        emit error(UnknownError);
                    }
                    else {
                        emit statusChanged(CaptchaRequired);
                    }
                }
            }
        }
    }

    reply->deleteLater();
}

void HugeFiles::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data;
    
    if (challenge == "captcha_code") {
        data = QString("op=download1&id=%1&fname=%2&rand=%3&method_free=Free Download&ctype=%4&code=%5")
                .arg(m_fileId).arg(m_fileName).arg(m_rand).arg(m_captchaType).arg(response);
    }
    else if (m_captchaService == "SolveMedia") {
        data = QString("op=download1&id=%1&fname=%2&rand=%3&method_free=Free Download&&ctype=%4&adcopy_challenge=%5&adcopy_response=manual_challenge")
                .arg(m_fileId).arg(m_fileName).arg(m_rand).arg(m_captchaType).arg(challenge);
    }
    else {
        data = QString("op=download1&id=%1&fname=%2&rand=%3&method_free=Free Download&&ctype=%4&recaptcha_challenge_field=%5&recaptcha_response_field=%6")
                .arg(m_fileId).arg(m_fileName).arg(m_rand).arg(m_captchaType).arg(challenge).arg(response);
    }

    QNetworkRequest request;
    request.setUrl(QUrl("http://hugefiles.net/" + m_fileId));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void HugeFiles::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http(s|)://\\d+\\.\\d+\\.\\d+\\.\\d+:\\d+/d/\\w+/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else {
        QString response(reply->readAll());

        if (response.contains("down_direct")) {
            this->getDownloadUrl();
        }
        else {
            if (response.contains("class=\"captcha_code\"")) {
                QString codeBlock = response.section("<td align=right>", 1, 1).section("</span></div>", 0, 0);
                QStringList codeList = codeBlock.split("padding-left:", QString::SkipEmptyParts);

                if (!codeList.isEmpty()) {
                    codeList.removeFirst();

                    QMap<int, QString> codeMap;

                    int i = 1;

                    while ((!codeList.isEmpty()) && (i < 5)) {
                        QString code = codeList.takeFirst();
                        int key = code.section('p', 0, 0).toInt();
                        QString value = QString::number(code.section(">&#", 1, 1).left(2).toInt() - 48);
                        codeMap[key] = value;
                        i++;
                    }

                    QString captchaCode;

                    foreach (QString val, codeMap.values()) {
                        captchaCode.append(val);
                    }

                    this->submitCaptchaResponse("captcha_code", captchaCode);
                }
                else {
                    emit error(UnknownError);
                }
            }
            else {
                if (response.contains("api.solvemedia.com")) {
                    m_captchaService = "SolveMedia";
                    m_captchaKey = response.section("api.solvemedia.com/papi/challenge.noscript?k=", 1, 1).section('"', 0, 0);
                }
                else if (response.contains("google.com/recaptcha")) {
                    m_captchaService = "Google";
                    m_captchaKey = response.section("google.com/recaptcha/api/challenge?k=", 1, 1).section('"', 0, 0);
                }

                if (m_captchaKey.isEmpty()) {
                    emit error(UnknownError);
                }
                else {
                    emit error(CaptchaError);
                }
            }
        }
    }

    reply->deleteLater();
}

void HugeFiles::getDownloadUrl() {
    QString data = QString("op=download2&id=%1&referer=http://hugefiles.net/%1&method_free=Free Download&down_direct=1").arg(m_fileId);
    QNetworkRequest request;
    request.setUrl(QUrl("http://hugefiles.net/" + m_fileId));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadUrl()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void HugeFiles::checkDownloadUrl() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (redirect.isEmpty()) {
        redirect = reply->header(QNetworkRequest::LocationHeader).toUrl();
    }

    if (redirect.isEmpty()) {
        emit error(UnknownError);
    }
    else {
        emit downloadRequestReady(QNetworkRequest(redirect));
    }

    reply->deleteLater();
}

bool HugeFiles::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(hugefiles, HugeFiles)
