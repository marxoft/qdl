#include "xvideos.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

XVideos::XVideos(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp XVideos::urlPattern() const {
    return QRegExp("http(s|)://(www.|)xvideos.com/video\\d+", Qt::CaseInsensitive);
}

bool XVideos::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void XVideos::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void XVideos::checkUrlIsValid() {
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
        QString result = response.section("flv_url=", 1, 1).section("&amp;", 0, 0);
        QString videoUrl(QByteArray::fromPercentEncoding(result.toUtf8()));

        if (videoUrl.startsWith("http")) {
            QString fileName = response.section("<title>", 1, 1).section('<', 0, 0) + ".flv";
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void XVideos::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void XVideos::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString result = response.section("flv_url=", 1, 1).section("&amp;", 0, 0);
    QString videoUrl(QByteArray::fromPercentEncoding(result.toUtf8()));

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

bool XVideos::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(xvideos, XVideos)
