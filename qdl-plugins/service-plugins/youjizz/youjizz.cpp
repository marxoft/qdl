#include "youjizz.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

YouJizz::YouJizz(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp YouJizz::urlPattern() const {
    return QRegExp("http(s|)://(www.|)youjizz.com/videos/\\.+", Qt::CaseInsensitive);
}

bool YouJizz::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void YouJizz::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void YouJizz::checkUrlIsValid() {
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
        QString videoUrl = response.section("file\",encodeURIComponent(\"", 1, 1).section('"', 0, 0);

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

void YouJizz::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void YouJizz::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString videoUrl = response.section("file\",encodeURIComponent(\"", 1, 1).section('"', 0, 0);

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

bool YouJizz::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(youjizz, YouJizz)
