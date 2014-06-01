#include "uploadable.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

using namespace QtJson;

Uploadable::Uploadable(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Uploadable::urlPattern() const {
    return QRegExp("http(s|)://(www.|)uploadable.ch/file/\\w+", Qt::CaseInsensitive);
}

bool Uploadable::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Uploadable::login(const QString &username, const QString &password) {
    QString data = QString("userName=%1&userPassword=%2").arg(username).arg(password);
    QUrl url("http://uploadable.ch/login.php");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploadable::checkLogin() {
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

void Uploadable::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploadable::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://adl.uploadable.ch/file/\\w+/[-\\w]+/[-\\w]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("file_name\" title=\"", 1, 1).section('"', 0, 0);

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void Uploadable::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploadable::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://adl.uploadable.ch/file/\\w+/[-\\w]+/[-\\w]+");
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
            m_fileId = response.section("recaptcha_shortencode_field\" value=\"", 1, 1).section('"', 0, 0);
            m_captchaKey = response.section("reCAPTCHA_publickey='", 1, 1).section('\'', 0, 0);

            if ((m_fileId.isEmpty()) || (m_captchaKey.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                this->getWaitTime();
            }
        }
    }

    reply->deleteLater();
}

void Uploadable::getWaitTime() {
    QUrl url("http://www.uploadable.ch/file/" + m_fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "text/javascript, text/html, application/xml, text/xml, */*");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Referer", "http://www.uploadable.ch/file/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, "downloadLink=wait");
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploadable::checkWaitTime() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap result = Json::parse(response).toMap();
    int secs = result.value("waitTime").toInt();

    if (secs > 0) {
        this->startWait(secs * 1000);

        if (secs > 30) {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadCheck()));
        }
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void Uploadable::getDownloadCheck() {
    QUrl url("http://www.uploadable.ch/file/" + m_fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "text/javascript, text/html, application/xml, text/xml, */*");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Referer", "http://www.uploadable.ch/file/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, "checkDownload=check");
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadCheck()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadCheck()));
}

void Uploadable::checkDownloadCheck() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap result = Json::parse(response).toMap();
    QString operation = result.value("success").toString();

    if (operation == "showCaptcha") {
        emit statusChanged(CaptchaRequired);
    }
    else {
        QString errorString = result.value("fail").toString();
        
        if (errorString == "timeLimit") {
            this->startWait(600000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void Uploadable::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("recaptcha_challenge_field=%1&recaptcha_response_field=%2&recaptcha_shortencode_field=%3").arg(challenge).arg(response).arg(m_fileId);
    QUrl url("http://www.uploadable.ch/checkReCaptcha.php");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "text/javascript, text/html, application/xml, text/xml, */*");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Referer", "http://www.uploadable.ch/file/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploadable::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap result = Json::parse(response).toMap();
    int success = result.value("success").toInt();

    if (success) {
        this->getDownloadLink();
    }
    else {
        QString errorString = result.value("error").toString();

        if (errorString == "incorrect-captcha-sol") {
            emit error(CaptchaError);
        }
        else if (errorString == "timeLimit") {
            this->startWait(600000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void Uploadable::getDownloadLink() {
    QUrl url("http://www.uploadable.ch/file/" + m_fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "text/javascript, text/html, application/xml, text/xml, */*");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Referer", "http://www.uploadable.ch/file/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, "downloadLink=show");
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploadable::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    this->getRedirect();

    reply->deleteLater();
}

void Uploadable::getRedirect() {
    QUrl url("http://www.uploadable.ch/file/" + m_fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "text/javascript, text/html, application/xml, text/xml, */*");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Referer", "http://www.uploadable.ch/file/" + m_fileId.toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, "download=normal");
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkRedirect()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploadable::checkRedirect() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (redirect.isEmpty()) {
        emit error(UnknownError);
    }
    else {
        emit downloadRequestReady(QNetworkRequest(redirect));
    }

    reply->deleteLater();
}

void Uploadable::startWait(int msecs) {
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

void Uploadable::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Uploadable::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Uploadable::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(uploadable, Uploadable)
