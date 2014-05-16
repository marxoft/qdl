#include "babevideo.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

BabeVideo::BabeVideo(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp BabeVideo::urlPattern() const {
    return QRegExp("http(s|)://(www.|)babevideo.co.uk/play/[\\w-_]+", Qt::CaseInsensitive);
}

bool BabeVideo::urlSupported(const QUrl &url) const {
    return urlPattern().indexIn(url.toString()) == 0;
}

void BabeVideo::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BabeVideo::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (!redirect.isEmpty()) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString videoUrl = response.left(response.indexOf("\">Download")).section('"', -1);

        if (videoUrl.startsWith("http")) {
            QString fileName = videoUrl.mid(videoUrl.lastIndexOf('_') + 1);
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void BabeVideo::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void BabeVideo::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString videoUrl = response.left(response.indexOf("\">Download")).section('"', -1);

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

bool BabeVideo::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(babevideo, BabeVideo)
