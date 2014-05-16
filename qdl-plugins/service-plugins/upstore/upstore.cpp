#include "upstore.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>
#include <QStringList>

Upstore::Upstore(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Upstore::urlPattern() const {
    return QRegExp("http(s|)://(www.|)(upsto.re|upstore.net)/\\w+", Qt::CaseInsensitive);
}

bool Upstore::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Upstore::login(const QString &username, const QString &password) {
    QString data = QString("send=Login&email=%1&password=%2").arg(username).arg(password);
    QUrl url("http://upstore.net/account/login/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Upstore::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
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

void Upstore::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Upstore::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+.(upsto.re|upstore.net)/\\w+/\\d+.\\d+.\\d+.\\d+[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("class=\"error\">")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("\"margin:0\">", 1, 1).section("<", 0, 0);

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

void Upstore::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QString urlString = webUrl.toString();

    if (urlString.split('/', QString::SkipEmptyParts).size() > 3) {
        urlString = urlString.section('/', 0, -2);
    }

    m_url = QUrl(urlString);
    QNetworkRequest request(m_url);
    QNetworkReply *reply = this->networkAccessManager()->post(request, "hash=" + urlString.section('/', -1).toUtf8() + "&free=Slow download");
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Upstore::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.(upsto.re|upstore.net)/\\w+/\\d+.\\d+.\\d+.\\d+[^'\"]+");
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
            int mins = response.section("Please wait", 1, 1).section("minutes before downloading", 0, 0).trimmed().toInt();
            int secs = 0;

            if (mins > 0) {
                secs = mins * 60;
            }
            else {
                secs = response.section("var sec = ", 1, 1).section(',', 0, 0).toInt();
            }

            m_captchaKey = response.section("Recaptcha.create('", 1, 1).section('\'', 0, 0);

            if ((secs <= 0) || (m_captchaKey.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                this->startWait(secs * 1000);

                if (secs > 60000) {
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

void Upstore::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void Upstore::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("kpw=spam&antispam=spam&free=Get download link&recaptcha_challenge_field=%1&recaptcha_response_field=%2&hash=%3").arg(challenge).arg(response).arg(m_url.toString().section('/', -1));
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Upstore::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.(upsto.re|upstore.net)/\\w+/\\d+.\\d+.\\d+.\\d+[^'\"]+");
    QString response(reply->readAll());

    if (re.indexIn(response) >= 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else if (response.contains("Wrong captcha code")) {
        emit error(CaptchaError);
    }
    else {
        int mins = response.section("Please wait", 1, 1).section("minutes before downloading", 0, 0).trimmed().toInt();

        if (mins > 0) {
            this->startWait(mins * 60000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void Upstore::startWait(int msecs) {
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

void Upstore::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Upstore::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Upstore::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(upstore, Upstore)
