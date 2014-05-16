#include "videobam.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QRegExp>

using namespace QtJson;

VideoBam::VideoBam(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp VideoBam::urlPattern() const {
    return QRegExp("http(s|)://(www.|)videobam.com/(videos/download/|)\\w+", Qt::CaseInsensitive);
}

bool VideoBam::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void VideoBam::checkUrl(const QUrl &webUrl) {
    QString id = webUrl.toString().section("/custom", 0, 0).section('/', -1);
    QUrl url("http://videobam.com/videos/download/" + id);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void VideoBam::checkUrlIsValid() {
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
        QString imageLink = response.section("\"og:image\" content=\"", 1).section('"', 0, 0);

        if (imageLink.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("\"og:title\" content=\"", 1).section('"', 0, 0);

            if (fileName.isEmpty()) {
                fileName = reply->request().url().toString().section('/', -1);
            }

            emit urlChecked(true, reply->request().url(), serviceName(), fileName + ".mp4");
        }
    }

    reply->deleteLater();
}

void VideoBam::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageLoaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void VideoBam::onWebPageLoaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString link = response.section("var url = '", 1, 1).section('\'', 0, 0);

    if (link.isEmpty()) {
        emit error(UnknownError);
    }
    else {
        QUrl url("http://videobam.com" + link);
        this->getDownloadLink(url);
    }

    reply->deleteLater();
}

void VideoBam::getDownloadLink(const QUrl &url) {
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadLink()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void VideoBam::checkDownloadLink() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap map = Json::parse(response).toMap();
    QUrl url = map.value("url").toUrl();

    if (url.isValid()) {
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else if (map.value("error").toString() == "You can not download more than 1 video per 30 minutes.") {
        this->startWait(600000);
        this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void VideoBam::startWait(int msecs) {
    if (msecs > 300000) {
        emit statusChanged(LongWait);
    }
    else {
        emit statusChanged(ShortWait);
    }

    emit waiting(msecs);
    m_waitTime = msecs;
    m_waitTimer->start(1000);
}

void VideoBam::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void VideoBam::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool VideoBam::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(videobam, VideoBam)
