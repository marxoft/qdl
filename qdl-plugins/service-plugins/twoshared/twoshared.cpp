#include "twoshared.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QTimer>

TwoShared::TwoShared(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp TwoShared::urlPattern() const {
    return QRegExp("http(s|)://(www.|)2shared.com/(file|document|fadmin|photo|audio|video)/\\w+", Qt::CaseInsensitive);
}

bool TwoShared::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void TwoShared::login(const QString &username, const QString &password) {
    QString data = QString("login=%1&password=%2").arg(username).arg(password);
    QUrl url("http://2shared.com/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TwoShared::checkLogin() {
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

void TwoShared::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TwoShared::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+.2shared.com/download/\\w+/.+\\?tsid=[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("file link that you requested is not valid")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("<title>", 1, 1).section(" download - 2shared", 0, 0);

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

void TwoShared::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TwoShared::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.2shared.com/download/\\w+/.+\\?tsid=[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        m_downloadUrl = QUrl(re.cap());
        this->getDownloadLink();
    }
    else if (!redirect.isEmpty()) {
        this->getDownloadRequest(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            m_downloadUrl = QUrl(re.cap());
            this->getDownloadLink();
        }
        else {
            int mins = response.section("id=\"timeToWait\"", 1, 1).section('"', 0, 0).toInt();

            if (mins > 0) {
                emit startWait(mins * 60000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
            }
            else {
                emit error(UnknownError);
            }
        }
    }

    reply->deleteLater();
}

void TwoShared::startWait(int msecs) {
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

void TwoShared::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void TwoShared::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

void TwoShared::getDownloadLink() {
    QNetworkRequest request(m_downloadUrl);
    QNetworkReply *reply = this->networkAccessManager()->head(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(getDownloadLink()));
}

void TwoShared::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (redirect.hasQueryItem("cau2")) {
        emit error(NotFound);
    }
    else {
        emit downloadRequestReady(QNetworkRequest(m_downloadUrl));
    }

    reply->deleteLater();
}

bool TwoShared::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(twoshared, TwoShared)
