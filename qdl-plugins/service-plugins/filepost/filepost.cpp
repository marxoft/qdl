#include "filepost.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

static const QString SESSION_ID("28d3724ebe3cc77cc2088727c5b184bb");
static const QString JS_HTTP_REQUEST("14009408677681-xml");
static const QString TOKEN("fl504ee47586080");

using namespace QtJson;

FilePost::FilePost(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp FilePost::urlPattern() const {
    return QRegExp("(http(s|)://(www.|)filepost.com/files/|http://fp.io/)\\w+", Qt::CaseInsensitive);
}

bool FilePost::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void FilePost::login(const QString &username, const QString &password) {
    QUrl url("http://filepost.com/general/login_form/");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("SID", SESSION_ID);
    query.addQueryItem("JsHttpRequest", JS_HTTP_REQUEST);
    url.setQuery(query);
#else
    url.addQueryItem("SID", SESSION_ID);
    url.addQueryItem("JsHttpRequest", JS_HTTP_REQUEST);
#endif
    QString data = QString("email=%1&password=%2&remember=on&recaptcha_response_field=&token=%3").arg(username).arg(password).arg(TOKEN);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilePost::checkLogin() {
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

void FilePost::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilePost::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http(s|)://fs\\d+.filepost.com/get_file/[^']+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("file_info file_info_deleted")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("<h1 title=\"", 1, 1).section('"', 0, 0);
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void FilePost::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilePost::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http(s|)://fs\\d+.filepost.com/get_file/[^']+");
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
        else if (response.contains("file_info file_info_deleted")) {
            emit error(NotFound);
        }
        else {
            m_captchaKey = response.section(QRegExp("key:\\s+'"), 1, 1).section('\'', 0, 0);
            m_code = response.section("name=\"code\" value=\"", 1, 1).section('"', 0, 0);

            if ((m_captchaKey.isEmpty()) || (m_code.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                this->getWaitTime();
            }
        }
    }

    reply->deleteLater();
}

void FilePost::getWaitTime() {
    QUrl url("http://filepost.com/files/get/");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("SID", SESSION_ID);
    query.addQueryItem("JsHttpRequest", JS_HTTP_REQUEST);
    url.setQuery(query);
#else
    url.addQueryItem("SID", SESSION_ID);
    url.addQueryItem("JsHttpRequest", JS_HTTP_REQUEST);
#endif
    QString data = QString("action=set_download&code=%1&token=%2").arg(m_code).arg(TOKEN);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWaitTime()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilePost::checkWaitTime() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    bool ok = false;
    QVariantMap map = Json::parse(response, ok).toMap();

    if (ok) {
        QVariantMap answer = map.value("js").toMap().value("answer").toMap();
        QVariantMap params = answer.value("params").toMap();

        int secs = 0;

        if (!params.isEmpty()) {
            secs = params.value("next_download").toInt();
        }
        else {
            secs = answer.value("wait_time").toInt();
        }

        if (secs > 0) {
            this->startWait(secs * 1000);

            if (secs > 90) {
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
            }
            else {
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
            }
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

void FilePost::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void FilePost::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QUrl url("http://filepost.com/files/get/");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("SID", SESSION_ID);
    query.addQueryItem("JsHttpRequest", JS_HTTP_REQUEST);
    url.setQuery(query);
#else
    url.addQueryItem("SID", SESSION_ID);
    url.addQueryItem("JsHttpRequest", JS_HTTP_REQUEST);
#endif
    QString data = QString("code=%1&file_pass=&recaptcha_challenge_field=%2&recaptcha_response_field=%3&token=%4").arg(m_code).arg(challenge).arg(response).arg(TOKEN);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FilePost::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());

    bool ok = false;
    QVariantMap map = Json::parse(response, ok).toMap();

    if (ok) {
        QUrl url = map.value("js").toMap().value("answer").toMap().value("link").toUrl();

        if (!url.isValid()) {
            QString errorString = map.value("error").toString();

            if (errorString == "You entered a wrong CAPTCHA code. Please try again.") {
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
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void FilePost::startWait(int msecs) {
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

void FilePost::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void FilePost::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool FilePost::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(filepost, FilePost)
