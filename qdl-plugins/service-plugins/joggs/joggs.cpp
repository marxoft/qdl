#include "joggs.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

Joggs::Joggs(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp Joggs::urlPattern() const {
    return QRegExp("http(s|)://(www.|)joggs.com/videos/[\\w-]+", Qt::CaseInsensitive);
}

bool Joggs::urlSupported(const QUrl &url) const {
    return urlPattern().indexIn(url.toString()) == 0;
}

void Joggs::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Joggs::checkUrlIsValid() {
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
        QString videoUrl = response.section("file=", 1, 1).section('&', 0, 0);

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

void Joggs::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Joggs::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString videoUrl = response.section("file=", 1, 1).section('&', 0, 0);

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

bool Joggs::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(joggs, Joggs)
