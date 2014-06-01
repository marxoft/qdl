#include "uploading.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

using namespace QtJson;

Uploading::Uploading(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Uploading::urlPattern() const {
    return QRegExp("http(s|)://(www.|)uploading.com/files/\\w+", Qt::CaseInsensitive);
}

bool Uploading::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Uploading::login(const QString &username, const QString &password) {
    QString data = QString("email=%1&password=%2").arg(username).arg(password);
    QUrl url("http://uploading.com/general/login_form");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploading::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    emit loggedIn((statusCode == 200) || (statusCode == 302));

    reply->deleteLater();
}

void Uploading::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploading::checkUrlIsValid() {
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
        QString response(reply->readAll());

        if (response.contains("File not found")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("keywords\" content=\"", 1, 1).section(',', 0, 0);

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

void Uploading::getDownloadRequest(const QUrl &webUrl) {
    m_url = webUrl;
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploading::onWebPageDownloaded() {
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
            m_fileId = response.section("code: \"", 1, 1).section('"', 0, 0);

            if (m_fileId.isEmpty()) {
                emit error(UnknownError);
            }
            else {
                this->getWaitTime();
            }
        }
    }

    reply->deleteLater();
}

void Uploading::getWaitTime() {
    QUrl url("http://uploading.com/files/get/?ajax");
    QString data = QString("code=%1&action=second_page").arg(m_fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploading::checkWaitTime() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap map = Json::parse(response).toMap();
    QVariantMap answer = map.value("answer").toMap();
    int secs = answer.value("wait_time").toInt();

    if (secs > 0) {
        this->startWait(secs * 1000);

        if (secs > 30) {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
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

void Uploading::getDownloadLink() {
    QUrl url("http://uploading.com/files/get/?ajax");
    QString data = QString("code=%1&action=get_link&pass=false").arg(m_fileId);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploading::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap map = Json::parse(response).toMap();
    QVariantMap answer = map.value("answer").toMap();
    QUrl url = answer.value("link").toUrl();

    if (url.isValid()) {
        this->getDownloadPage(url);
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void Uploading::getDownloadPage(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uploading::checkDownloadPage() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QRegExp re("http://fs\\d+\\.uploading.com/get_file/[^\"]+");

    if (re.indexIn(response) >= 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void Uploading::startWait(int msecs) {
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

void Uploading::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Uploading::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Uploading::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(uploading, Uploading)
