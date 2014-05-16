#include "metacafe.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>

using namespace QtJson;

Metacafe::Metacafe(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp Metacafe::urlPattern() const {
    return QRegExp("http(s|)://(www.|)metacafe.com/watch/\\w+", Qt::CaseInsensitive);
}

bool Metacafe::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Metacafe::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Metacafe::checkUrlIsValid() {
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
        QString response(QByteArray::fromPercentEncoding(reply->readAll()));
        QVariantMap mediaData = Json::parse(response.section("mediaData=", 1, 1).section("&postRollContentURL", 0, 0)).toMap();
        
        if (!mediaData.isEmpty()) {
            QString fileName = response.section("og:title\" content=\"", 1, 1).section('"', 0, 0) + ".mp4";
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void Metacafe::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Metacafe::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }
    
    QString response(QByteArray::fromPercentEncoding(reply->readAll()));
    QVariantMap mediaData = Json::parse(response.section("mediaData=", 1, 1).section("&postRollContentURL", 0, 0)).toMap();
        
    if (!mediaData.isEmpty()) {
        QString format = QSettings("QDL", "QDL").value("Metacafe/videoFormat", "MP4").toString();
        QUrl url(QUrl::fromPercentEncoding(mediaData.value(format).toMap().value("mediaURL").toByteArray()));
        
        if (url.isValid()) {
            emit downloadRequestReady(QNetworkRequest(url));
        }
        else if (format != "MP4") {
            url.setUrl(QUrl::fromPercentEncoding(mediaData.value("MP4").toMap().value("mediaURL").toByteArray()));
            
            if (url.isValid()) {
                emit downloadRequestReady(QNetworkRequest(url));
            }
            else {
                emit error(UnknownError);
            }
        }
        else {
            emit error(UnknownError);
        }        
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

bool Metacafe::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(metacafe, Metacafe)
