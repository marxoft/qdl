#include "cramit.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

Cramit::Cramit(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Cramit::urlPattern() const {
    return QRegExp("http(s|)://(www.|)cramit.in/\\w+", Qt::CaseInsensitive);
}

bool Cramit::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Cramit::login(const QString &username, const QString &password) {
    QString data = QString("op=login&redirect=&login=%1&password=%2&x=0&y=0").arg(username).arg(password);
    QUrl url("http://cramit.in/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Cramit::checkLogin() {
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

void Cramit::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Cramit::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://cramit.in/file_download/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("File Not Found")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section(QRegExp("<title>.+Download File "), 1, 1).section('<', 0, 0);

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

void Cramit::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_fileId = webUrl.toString().section('/', -1);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Cramit::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://cramit.in/file_download/[^'\"]+");
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
        else if (response.contains("File Not Found")) {
            emit error(NotFound);
        }
        else {
            this->getWaitTime();
        }
    }

    reply->deleteLater();
}

void Cramit::getWaitTime() {
    QUrl url(QString("http://cramit.in/%1").arg(m_fileId));
    QString data = QString("op=download1&id=%1&method_free=FREE DOWNLOAD").arg(m_fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Cramit::checkWaitTime() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    if (response.contains("You have to wait")) {
        int mins = response.section("You have to wait ", 1, 1).section(" minutes", 0, 0).toInt();
        int secs = response.section(" seconds before your next download", 0, 0).section(' ', 1, 1).toInt();
        this->startWait((mins * 60000) + (secs + 1000));
        this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
    }
    else if (response.contains("Enter the code below")) {
        m_captchaKey = response.section("rand\" value=\"", 1, 1).section('"', 0, 0);

        if (m_captchaKey.isEmpty()) {
            emit error(UnknownError);
        }
        else {
            emit statusChanged(CaptchaRequired);
        }
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void Cramit::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QUrl url(QString("http://cramit.in/%1").arg(m_fileId));
    QString data = QString("op=download2&id=%1&method_free=FREE DOWNLOAD&down_direct=1&rand=%2&code=%3").arg(m_fileId).arg(challenge).arg(response);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Cramit::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://cramit.in/file_download/[^'\"]+");
    QString response(reply->readAll());

    if (re.indexIn(response) >= 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else if (response.contains("Wrong captcha")) {
        emit error(CaptchaError);
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void Cramit::startWait(int msecs) {
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

void Cramit::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Cramit::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Cramit::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(cramit, Cramit)
