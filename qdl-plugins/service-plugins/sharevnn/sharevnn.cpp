#include "sharevnn.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

ShareVnn::ShareVnn(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp ShareVnn::urlPattern() const {
    return QRegExp("http(s|)://(www.|)share.vnn.vn/\\w+", Qt::CaseInsensitive);
}

bool ShareVnn::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void ShareVnn::login(const QString &username, const QString &password) {
    QString data = QString("username=%1&password=%2").arg(username).arg(password);
    QUrl url("http://share.vnn.vn/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ShareVnn::checkLogin() {
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

void ShareVnn::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ShareVnn::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http(s|)://.+/dl\\d+/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll().simplified());
        QString fileName = response.section("dl-file\"> <h1>", 1, 1).section('<', 0, 0).trimmed();

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void ShareVnn::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_url = webUrl;
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ShareVnn::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http(s|)://.+/dl\\d+/[^'\"]+");
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
            m_fileId = response.section("file_id\" rel=\"", 1, 1).section('"', 0, 0);
            int secs = response.section("timewait' value='", 1, 1).section('\'', 0, 0).toInt();

            if ((m_fileId.isEmpty()) || (secs <= 0)) {
                emit error(UnknownError);
            }
            else {
                this->startWait(secs * 1000);

                if (secs > 60) {
                    this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
                }
                else {
                    this->connect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
                }
            }
        }
    }

    reply->deleteLater();
}

void ShareVnn::getDownloadLink() {
    QUrl url("http://share.vnn.vn/ajax.php");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("pkg", "download");
    query.addQueryItem("fnc", "getlink");
    query.addQueryItem("id", m_fileId);
    url.setQuery(url);
#else
    url.addQueryItem("pkg", "download");
    url.addQueryItem("fnc", "getlink");
    url.addQueryItem("id", m_fileId);
#endif
    QNetworkRequest request(url);
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ShareVnn::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http(s|)://.+/dl\\d+/[^'\"]+");
    QString response(reply->readAll());

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

void ShareVnn::startWait(int msecs) {
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

void ShareVnn::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void ShareVnn::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool ShareVnn::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(sharevnn, ShareVnn)
