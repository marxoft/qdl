#include "bestreams.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QTimer>

Bestreams::Bestreams(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Bestreams::urlPattern() const {
    return QRegExp("http(s|)://(www.|)bestreams.net/\\w+", Qt::CaseInsensitive);
}

bool Bestreams::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Bestreams::login(const QString &username, const QString &password) {
    QString data = QString("op=login&login=%1&password=%2").arg(username).arg(password);
    QUrl url("http://bestreams.net/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Bestreams::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    emit loggedIn((statusCode == 200) || (statusCode == 302));

    reply->deleteLater();
}

void Bestreams::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Bestreams::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
        this->checkUrl(redirect);
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("fname\" value=\"", 1, 1).section('"', 0, 0);

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void Bestreams::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Bestreams::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
        this->getDownloadRequest(redirect);
    }
    else {
        QString response(reply->readAll());
        m_url = reply->request().url();
        m_id = response.section("id\" value=\"", 1, 1).section('"', 0, 0);
        m_fname = response.section("fname\" value=\"", 1, 1).section('"', 0, 0);
        m_hash = response.section("hash\" value=\"", 1, 1).section('"', 0, 0);
        m_human = response.section("imhuman\" value=\"", 1, 1).section('"', 0, 0);

        if ((m_url.isEmpty()) || (m_id.isEmpty()) || (m_fname.isEmpty()) || (m_hash.isEmpty()) || (m_human.isEmpty())) {
            emit error(UnknownError);
        }
        else {
            int waitTime = response.section("<span id=\"cxc\">", 1, 1).section('<', 0, 0).toInt();

            if (waitTime > 5) {
                this->startWait(waitTime * 1000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(checkPageTwo()));
            }
            else {
                QTimer::singleShot(waitTime * 1200, this, SLOT(getPageTwo()));
            }
        }
    }

    reply->deleteLater();
}

void Bestreams::getPageTwo() {
    QString data = QString("op=download1&usr_login=&referer=&id=%1&fname=%2&hash=%3&imhuman=%4").arg(m_id).arg(m_fname).arg(m_hash).arg(m_human);
    QNetworkRequest request(m_url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setRawHeader("Referer", m_url.toString().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkPageTwo()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(getPageTwo()));
}

void Bestreams::checkPageTwo() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
        this->getDownloadRequest(redirect);
    }
    else {
        QString response(reply->readAll());
        QString url = response.section("file: \"", 1, 1).section('"', 0, 0);

        if (url.isEmpty()) {
            emit error(UnknownError);
        }
        else {
            QNetworkRequest request;
            request.setUrl(QUrl(url));
            emit downloadRequestReady(request);
        }
    }

    reply->deleteLater();
}

void Bestreams::startWait(int msecs) {
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

void Bestreams::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Bestreams::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Bestreams::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(bestreams, Bestreams)
