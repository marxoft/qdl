#include "filesmonster.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

using namespace QtJson;

FilesMonster::FilesMonster(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp FilesMonster::urlPattern() const {
    return QRegExp("http(s|)://(www.|)filesmonster.com/download.php\\?id=\\w+", Qt::CaseInsensitive);
}

bool FilesMonster::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void FilesMonster::login(const QString &username, const QString &password) {
    QString data = QString("user=%1&pass=%2").arg(username).arg(password);
    QUrl url("http://filesmonster.com/login.php");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilesMonster::checkLogin() {
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

void FilesMonster::checkUrl(const QUrl &webUrl) {
    m_url = webUrl;
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilesMonster::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString multipartLink = response.section("action=\"/dl/", 1, 1).section('"', 0, 0);

    if (!multipartLink.isEmpty()) {
        QUrl multipartUrl("http://filesmonster.com/dl/" + multipartLink);
        this->getMultipartPage(multipartUrl);
    }
    else if (!m_fileName.isEmpty()) {
        int mins = response.section("will be available for download in ", 1, 1).section(" min", 0, 0).toInt();

        if (mins > 0) {
            this->startWait(mins * 60000);
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            emit error(UnknownError);
        }
    }
    else {
        emit urlChecked(false);
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void FilesMonster::getMultipartPage(const QUrl &url) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, QByteArray());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkMultipartPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilesMonster::checkMultipartPage() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString jsonLink = response.section("reserve_ticket('", 1, 1).section('\'', 0, 0);
    m_downloadLink = response.section("step2UrlTemplate = '", 1, 1).section('!', 0, 0);

    if ((!jsonLink.isEmpty()) && (!m_downloadLink.isEmpty())) {
        QUrl url("http://filesmonster.com" + jsonLink);
        this->getMultipartJson(url);
    }
    else {
        emit urlChecked(false);
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void FilesMonster::getMultipartJson(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkMultipartJson()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilesMonster::checkMultipartJson() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantList list = Json::parse(response).toList();

    if (list.isEmpty()) {
        emit urlChecked(false);
        emit error(UnknownError);
    }
    else {
        while (!list.isEmpty()) {
            QVariantMap map = list.takeFirst().toMap();
            QString code = map.value("dlcode").toString();
            QString fileName = map.value("name").toString();

            if ((!code.isEmpty()) && (!fileName.isEmpty())) {
                if (fileName == m_fileName) {
                    this->getDownloadRequest("http://filesmonster.com" + m_downloadLink + code + "/");
                }
                else if (m_fileName.isEmpty()) {
                    QUrl url(m_url);
#if QT_VERSION >= 0x050000
                    QUrlQuery query(url);
                    query.addQueryItem("fileName", fileName);
                    url.setQuery(query);
#else
                    url.addQueryItem("fileName", fileName);
#endif
                    emit urlChecked(true, url, this->serviceName(), fileName, list.isEmpty());
                }
            }
            else if (m_fileName.isEmpty()) {
                emit urlChecked(false, QUrl(), QString(), QString(), list.isEmpty());
            }
        }
    }

    reply->deleteLater();
}

void FilesMonster::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);

    if (webUrl.hasQueryItem("fileName")) {
        m_fileName = webUrl.queryItemValue("fileName");
        QUrl url(webUrl.toString().section('&', 0, -2));
        this->checkUrl(url);
    }
    else {
        m_url = webUrl;
        QNetworkRequest request(webUrl);
        request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
        QNetworkReply *reply = this->networkAccessManager()->get(request);
        this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
        this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    }
}

void FilesMonster::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+\\.uk.fmdepo.net/get/[-\\w]+");
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
            m_captchaKey = response.section("http://api.recaptcha.net/challenge?k=", 1, 1).section('&', 0, 0);

            if (m_captchaKey.isEmpty()) {
                emit error(UnknownError);
            }
            else {
                emit statusChanged(CaptchaRequired);
            }
        }
    }

    reply->deleteLater();
}

void FilesMonster::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("recaptcha_challenge_field=%1&recaptcha_response_field=%2").arg(challenge).arg(response);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilesMonster::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    int secs = response.section("id='sec'>", 1, 1).section('<', 0, 0).toInt();
    m_downloadLink = response.section("get_link('", 1, 1).section('\'', 0, 0);

    if ((secs > 0) && (!m_downloadLink.isEmpty())) {
        this->startWait(secs * 1000);

        if (secs > 30) {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
        }
        else {
            this->connect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
        }
    }
    else {
        QString errorString = response.section("class=\"error\">", 1, 1).section('<', 0, 0);

        if (errorString.startsWith("Wrong captcha text")) {
            emit error(CaptchaError);
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void FilesMonster::getDownloadLink() {
    QUrl url("http://filesmonster.com" + m_downloadLink);
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "text/javascript, text/html, application/xml, text/xml, */*");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilesMonster::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap map = Json::parse(response).toMap();
    QUrl url = map.value("url").toUrl();
    QString fileRequest = map.value("file_request").toString();

    if ((url.isValid()) && (!fileRequest.isEmpty())) {
        QString data("X-File-Request=" + fileRequest);
        emit downloadRequestReady(QNetworkRequest(url), data.toUtf8());
    }
    else {
        QString errorString = map.value("error").toString();

        if (errorString.startsWith("Link has expired")) {
            errorString.remove('\\');
            QUrl url(response.section("a href=\"", 1, 1).section('"', 0, 0));

            if ((url.isValid()) && (!m_fileName.isEmpty())) {
#if QT_VERSION >= 0x050000
                QUrlQuery query(url);
                query.addQueryItem("fileName", m_fileName);
                url.setQuery(query);
#else
                url.addQueryItem("fileName", m_fileName);
#endif
                this->getDownloadRequest(m_url);
            }
            else {
                emit error(UnknownError);
            }
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void FilesMonster::startWait(int msecs) {
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

void FilesMonster::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void FilesMonster::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool FilesMonster::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(filesmonster, FilesMonster)
