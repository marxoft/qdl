#include "rampanttv.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

RampantTV::RampantTV(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp RampantTV::urlPattern() const {
    return QRegExp("http://tube.rampant.tv/videos/\\w+", Qt::CaseInsensitive);
}

bool RampantTV::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void RampantTV::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void RampantTV::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

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
        QString paramsUrl = response.section("flashvars\",\"settings=", 1, 1).section('"', 0, 0);

        if (paramsUrl.startsWith("http")) {
            QString fileName = response.section("<h2>", 1, 1).section('<', 0, 0) + ".mp4";
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void RampantTV::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void RampantTV::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString paramsUrl = response.section("flashvars\",\"settings=", 1, 1).section('"', 0, 0);

    if (paramsUrl.startsWith("http")) {
        this->getVideoParams(QUrl(paramsUrl));
    }
    else {
        emit error(UrlError);
    }

    reply->deleteLater();
}

void RampantTV::getVideoParams(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoParams()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void RampantTV::parseVideoParams() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString videoUrl = response.section("defaultVideo:", 1, 1).section(';', 0, 0);

    if (videoUrl.startsWith("http")) {
        QNetworkRequest request;
        request.setUrl(QUrl(videoUrl));
        emit downloadRequestReady(request);
    }
    else {
        emit error(UrlError);
    }

    reply->deleteLater();
}

bool RampantTV::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(rampanttv, RampantTV)
