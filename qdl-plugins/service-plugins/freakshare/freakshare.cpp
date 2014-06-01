#include "freakshare.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

FreakShare::FreakShare(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp FreakShare::urlPattern() const {
    return QRegExp("http(s|)://(www.|)freakshare.com/(files/|\\?f=)\\w+", Qt::CaseInsensitive);
}

bool FreakShare::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void FreakShare::login(const QString &username, const QString &password) {
    QString data = QString("user=%1&password=%2&submit=Login").arg(username).arg(password);
    QUrl url("http://freakshare.com/login.html");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FreakShare::checkLogin() {
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

void FreakShare::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FreakShare::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+.freakshare.com/get.php\\?dlid=[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains(QRegExp("404 - Not Found|or is deleted|This file does not exist"))) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section(QRegExp("action=\"http://freakshare.com/files/\\w+/"), 1, 1).section(".html", 0, 0);

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

void FreakShare::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_url = webUrl;
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FreakShare::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.freakshare.com/get.php\\?dlid=[^'\"]+");
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
        else if (response.contains("Your Traffic is used up for today")) {
            this->startWait(300000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            m_did = response.section("\" name=\"did\"", 0, 0).section('"', -1);
            int secs = response.section("var time = ", 1, 1).section('.', 0, 0).toInt();

            if ((!m_did.isEmpty()) && (secs > 0)) {
                this->startWait(secs * 1000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
            }
            else {
                emit error(UnknownError);
            }
        }
    }

    reply->deleteLater();
}

void FreakShare::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void FreakShare::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("section=waitingtime&did=%1&recaptcha_challenge_field=%2&recaptcha_response_field=%3").arg(m_did).arg(challenge).arg(response);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FreakShare::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.freakshare.com/get.php\\?dlid=[^'\"]+");
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
        else if (response.contains("Wrong Captcha")) {
            emit error(CaptchaError);
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void FreakShare::startWait(int msecs) {
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

void FreakShare::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void FreakShare::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool FreakShare::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(freakshare, FreakShare)
