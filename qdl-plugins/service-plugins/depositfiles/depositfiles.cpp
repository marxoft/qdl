#include "depositfiles.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QScriptEngine>
#include <QTimer>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

DepositFiles::DepositFiles(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp DepositFiles::urlPattern() const {
    return QRegExp("http(s|)://(www.|)(depositfiles.com|dfiles.[a-z]+)/(\\w+/|)files/\\w+", Qt::CaseInsensitive);
}

bool DepositFiles::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void DepositFiles::login(const QString &username, const QString &password) {
    QString data = QString("login=%1&password=%2").arg(username).arg(password);
    QUrl url("http://depositfiles.com/api/user/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void DepositFiles::checkLogin() {
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

void DepositFiles::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void DepositFiles::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://fileshare\\d+.(depositfiles.com|dfiles.\\w+)/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

	if (response.contains("file does not exist")) {
	    emit urlChecked(false);
	}
	else {
            QScriptEngine engine;
            QString script = response.section("eval( ", 1, 1).section(");", 0, 0);
            QString fileNameElement = engine.evaluate(script).toString();

            if (fileNameElement.isEmpty()) {
                emit urlChecked(false);
            }
            else {
                QString fileName = fileNameElement.section("title=\"", 1, 1).section('"', 0, 0);
                emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
            }
	}
    }

    reply->deleteLater();
}

void DepositFiles::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, "gateway_result=1");
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void DepositFiles::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://fileshare\\d+.(depositfiles.com|dfiles.\\w+)/[^'\"]+");
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
        else if (response.contains("html_download_api-limit_interval")) {
            int secs = response.section("html_download_api-limit_interval\">", 1, 1).section('<', 0, 0).toInt();

            if (secs > 0) {
                this->startWait(secs * 1000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
            }
            else {
                emit error(UrlError);
            }
        }
        else {
            m_captchaKey = response.section("Recaptcha.create('", 1, 1).section('\'', 0, 0);
            m_fileId = response.section("var fid = '", 1, 1).section('\'', 0, 0);

            if ((m_captchaKey.isEmpty()) || (m_fileId.isEmpty())) {
		if (response.contains("file does not exist")) {
                    emit error(NotFound);
		}
		else {
		    emit error(UrlError);
		}
            }
            else {
                this->startWait(60000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
            }
        }
    }

    reply->deleteLater();
}

void DepositFiles::downloadCaptcha() {
    emit statusChanged(CaptchaRequired);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(downloadCaptcha()));
}

void DepositFiles::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QUrl url("http://depositfiles.com/get_file.php");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("fid", m_fileId);
    query.addQueryItem("challenge", challenge);
    query.addQueryItem("response", response);
    url.setQuery(query);
#else
    url.addQueryItem("fid", m_fileId);
    url.addQueryItem("challenge", challenge);
    url.addQueryItem("response", response);
#endif
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void DepositFiles::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://fileshare\\d+.(depositfiles.com|dfiles.\\w+)/[^'\"]+");
    QString response(reply->readAll());

    if (re.indexIn(response) >= 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else if (response.contains("check_recaptcha")) {
        emit error(CaptchaError);
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void DepositFiles::startWait(int msecs) {
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

void DepositFiles::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void DepositFiles::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool DepositFiles::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(depositfiles, DepositFiles)
