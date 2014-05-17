#include "uloz.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace QtJson;

Uloz::Uloz(QObject *parent) :
    RecaptchaPlugin(parent)
{
}

void Uloz::getCaptcha(const QString &key) {
    QUrl url("http://www.ulozto.net/reloadXapca.php?rnd=" + key);
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "text/javascript, text/html, application/xml, text/xml, */*");
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uloz::onCaptchaDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode != 200) {
        this->reportError(statusCode);
    }
    else {
        QString response(reply->readAll());
        QVariantMap result = Json::parse(response).toMap();
        QString image = result.value("image").toString();
        QString salt = result.value("salt").toString();
        QString timestamp = result.value("timestamp").toString();
        QString hash = result.value("hash").toString();
        
        if ((image.isEmpty()) || (salt.isEmpty()) || (timestamp.isEmpty()) || (hash.isEmpty())) {
            emit error(CaptchaNotFound);
        }
        else {
            this->setChallenge(QString("captcha_type=xapca&salt=%1&timestamp=%2&hash=%3").arg(salt).arg(timestamp).arg(hash));
            this->downloadCaptchaImage(QUrl(image));
        }
    }

    reply->deleteLater();
}

void Uloz::downloadCaptchaImage(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaImageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uloz::onCaptchaImageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 200) {
        emit captchaReady(reply->readAll());
    }
    else {
        this->reportError(statusCode);
    }

    reply->deleteLater();
}

void Uloz::reportError(int errorCode) {
    switch (errorCode) {
    case 404:
        emit error(CaptchaNotFound);
        break;
    case 503:
        emit error(ServiceUnavailable);
        break;
    case 500:
        emit error(InternalError);
        break;
    case 403:
        emit error(Unauthorised);
        break;
    default:
        emit error(UnknownError);
    }
}

bool Uloz::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(ulozrecaptcha, Uloz)
