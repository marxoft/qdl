#include "datafile.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

using namespace QtJson;

Datafile::Datafile(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Datafile::urlPattern() const {
    return QRegExp("http(s|)://(www.|)datafile.com/\\w/\\w+", Qt::CaseInsensitive);
}

bool Datafile::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Datafile::login(const QString &username, const QString &password) {
    QUrl url("https://www.datafile.com/login.html");
    QString data = QString("login=%1&password=%2&remember_me=1").arg(username).arg(password);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Datafile::checkLogin() {
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

void Datafile::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Datafile::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http(s|)://\\w\\d+.datafile.com/[^']+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("ErrorCode")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("file-name\">", 1, 1).section('<', 0, 0).trimmed();
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void Datafile::getDownloadRequest(const QUrl &webUrl) {
    m_url = webUrl;
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Datafile::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http(s|)://\\w\\d+.datafile.com/[^']+");
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
            m_captchaKey = response.section("challenge?k=", 1, 1).section('"', 0, 0);
            m_fileId = response.section("getFileDownloadLink('", 1, 1).section('\'', 0, 0);
            int secs = response.section("contdownTimer('", 1, 1).section('\'', 0, 0).toInt();

            if ((m_captchaKey.isEmpty()) || (m_fileId.isEmpty()) || (secs == 0)) {
                this->setErrorString(response.section(QRegExp("ErrorCode \\d+:"), 1, 1).section('<', 0, 0).trimmed());
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

void Datafile::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void Datafile::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("doaction=getFileDownloadLink&fileid=%1&recaptcha_challenge_field=%2&recaptcha_response_field=%3").arg(m_fileId).arg(challenge).arg(response);
    QUrl url("https://www.datafile.com/files/ajax.html");
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Datafile::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    response.remove(0, response.indexOf('{') - 1);

    bool ok = false;
    QVariantMap map = Json::parse(response, ok).toMap();

    if (ok) {
        QUrl url = map.value("link").toUrl();

        if (!url.isValid()) {
            this->setErrorString(map.value("msg").toString());

            if (this->errorString().contains("Please try again")) {
                emit error(CaptchaError);
            }
            else {
                emit error(UnknownError);
            }
        }
        else {
            emit downloadRequestReady(QNetworkRequest(url));
        }
    }
    else {
        this->setErrorString(QString());
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void Datafile::startWait(int msecs) {
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

void Datafile::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Datafile::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Datafile::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(datafile, Datafile)
