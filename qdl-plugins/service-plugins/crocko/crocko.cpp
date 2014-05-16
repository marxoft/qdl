#include "crocko.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

Crocko::Crocko(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Crocko::urlPattern() const {
    return QRegExp("http(s|)://(www.|)crocko.com/\\w+", Qt::CaseInsensitive);
}

bool Crocko::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Crocko::login(const QString &username, const QString &password) {
    QString data = QString("login=%1&password=%2&remember=1").arg(username).arg(password);
    QUrl url("http://www.crocko.com");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Crocko::checkLogin() {
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

void Crocko::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Crocko::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://fs\\d+\\.uploading.com/get_file/[^\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll().simplified());
        QString fileName = response.section("Download: <strong>", 1, 1).section('<', 0, 0).trimmed();

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void Crocko::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Crocko::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://fs\\d+\\.uploading.com/get_file/[^\"]+");
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
            int secs = response.section("w='", 1, 1).section('\'', 0, 0).toInt();
            m_fileId = response.section("u='/file_contents/captcha/", 1, 1).section('\'', 0, 0);

            if ((secs <= 0) || (m_fileId.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                this->startWait(secs * 1000);

                if (secs > 120) {
                    this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
                }
                else {
                    this->connect(this, SIGNAL(waitFinished()), this, SLOT(getCaptchaUrl()));
                }
            }
        }
    }

    reply->deleteLater();
}

void Crocko::getCaptchaUrl() {
    QUrl url("http://www.crocko.com/file_contents/captcha/" + m_fileId);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkCaptchaUrl()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(getCaptchaUrl()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Crocko::checkCaptchaUrl() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    if (response.contains("There is another download in progress")) {
        this->startWait(600000);
        this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
    }
    else {
        m_captchaUrl.setUrl(response.section("action=\"", 1, 1).section('"', 0, 0));
        m_captchaId = response.section("id\" value=\"", 1, 1).section('"', 0, 0);
        m_captchaKey = response.section("Recaptcha.create(\"", 1, 1).section('"', 0, 0);

        if ((!m_captchaUrl.isValid()) || (m_captchaId.isEmpty()) || (m_captchaKey.isEmpty())) {
            emit error(UnknownError);
        }
        else {
            emit statusChanged(CaptchaRequired);
        }
    }

    reply->deleteLater();
}

void Crocko::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("recaptcha_challenge_field=%1&recaptcha_response_field=%2&id=%3").arg(challenge).arg(response).arg(m_captchaId);
    QNetworkRequest request(m_captchaUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    emit downloadRequestReady(request, data.toUtf8());
}

void Crocko::startWait(int msecs) {
    if (msecs > 120000) {
        emit statusChanged(LongWait);
    }
    else {
        emit statusChanged(ShortWait);
    }

    emit waiting(msecs);
    m_waitTime = msecs;
    m_waitTimer->start(1000);
}

void Crocko::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Crocko::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Crocko::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(crocko, Crocko)
