#include "bitshare.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

BitShare::BitShare(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp BitShare::urlPattern() const {
    return QRegExp("http(s|)://(www.|)bitshare.com/(files/|\\?f=)\\w+", Qt::CaseInsensitive);
}

bool BitShare::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void BitShare::login(const QString &username, const QString &password) {
    QString data = QString("user=%1&password=%2&rememberlogin=&submit=Login").arg(username).arg(password);
    QUrl url("http://bitshare.com/login.html");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BitShare::checkLogin() {
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

void BitShare::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BitShare::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+.bitshare.com/download.php\\?d=\\w+&g=\\d");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("<h1>Error - File not available</h1>")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("<title>Download", 1, 1).section("- BitShare.com", 0, 0).trimmed();

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

void BitShare::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_fileId = webUrl.toString().section(QRegExp("files/|\\?f="), -1).section('/', 0, 0);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BitShare::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.bitshare.com/download.php\\?d=\\w+&g=\\d");
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
        else if (response.contains("you reached your hourly traffic limit", Qt::CaseInsensitive)) {
            int secs = response.section("var blocktime = ", 1, 1).section(';', 0, 0).toInt();
            this->startWait(secs * 1000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else if (response.contains("traffic is used up for today", Qt::CaseInsensitive)) {
            this->startWait(600000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            m_captchaKey = response.section("api.recaptcha.net/challenge?k=", 1, 1).section('"', 0, 0);
            m_ajaxDl = response.section("var ajaxdl = \"", 1, 1).section('"', 0, 0);

            if ((m_captchaKey.isEmpty()) || (m_ajaxDl.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                this->getWaitTime();
            }
        }
    }

    reply->deleteLater();
}

void BitShare::getWaitTime() {
    QUrl url(QString("http://bitshare.com/files-ajax/%1/request.html").arg(m_fileId));
    QString data = QString("request=generateID&ajaxid=%1").arg(m_ajaxDl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BitShare::checkWaitTime() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    int secs = response.section(':', 1, 1).toInt();
    int captchaRequired = response.section(':', -1).toInt();

    if (secs > 0) {
        this->startWait(secs * 1000);

        if (captchaRequired) {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
        }
        else {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
        }
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void BitShare::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void BitShare::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QUrl url(QString("http://bitshare.com/files-ajax/%1/request.html").arg(m_fileId));
    QString data = QString("request=validateCaptcha&ajaxid=%1&recaptcha_challenge_field=%2&recaptcha_response_field=%3").arg(m_ajaxDl).arg(challenge).arg(response);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BitShare::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    if (response.contains("success", Qt::CaseInsensitive)) {
        this->getDownloadLink();
    }
    else if (response.contains("incorrect-captcha-sol", Qt::CaseInsensitive)) {
        emit error(CaptchaError);
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void BitShare::getDownloadLink() {
    QUrl url(QString("http://bitshare.com/files-ajax/%1/request.html").arg(m_fileId));
    QString data = QString("request=getDownloadURL&ajaxid=%1").arg(m_ajaxDl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BitShare::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    if (response.contains("success", Qt::CaseInsensitive)) {
	QUrl url(response.section('#', 1, 1));

	if (url.isValid()) {
	    emit downloadRequestReady(QNetworkRequest(url));
	}
	else {
	    emit error(UnknownError);
	}
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void BitShare::startWait(int msecs) {
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

void BitShare::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void BitShare::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool BitShare::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(bitshare, BitShare)
