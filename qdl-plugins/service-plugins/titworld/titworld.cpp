#include "titworld.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

TitWorld::TitWorld(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp TitWorld::urlPattern() const {
    return QRegExp("http(s|)://(www.|)titworld.net/\\d+", Qt::CaseInsensitive);
}

bool TitWorld::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void TitWorld::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TitWorld::checkUrlIsValid() {
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
        QString url = response.section("'file','", 1, 1).section('\'', 0, 0);

        if (url.startsWith("http")) {
            QString fileName = response.section("'title','", 1, 1).section('\'', 0, 0) + ".flv";
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void TitWorld::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void TitWorld::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString url = response.section("'file','", 1, 1).section('\'', 0, 0);

    if (url.startsWith("http")) {
        QNetworkRequest request;
        request.setUrl(QUrl(url));
        emit downloadRequestReady(request);
    }
    else {
        emit error(UrlError);
    }

    reply->deleteLater();
}

bool TitWorld::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(titworld, TitWorld)
