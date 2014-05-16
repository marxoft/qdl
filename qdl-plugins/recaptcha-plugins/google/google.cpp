#include "google.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

Google::Google(QObject *parent) :
    RecaptchaPlugin(parent)
{
}

void Google::getCaptcha(const QString &key) {
    this->setKey(key);
    QUrl url("http://www.google.com/recaptcha/api/challenge?k=" + key);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Google::onCaptchaDownloaded() {
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
        QString challenge = response.section("challenge : '", 1, 1).section("'", 0, 0);

        if (challenge.isEmpty()) {
            emit error(CaptchaNotFound);
        }
        else {
            this->downloadCaptchaImage(challenge);
        }
    }

    reply->deleteLater();
}

void Google::downloadCaptchaImage(const QString &challenge) {
    this->setChallenge(challenge);
    QUrl url("http://www.google.com/recaptcha/api/image?c=" + challenge);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaImageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Google::onCaptchaImageDownloaded() {
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

void Google::reportError(int errorCode) {
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

bool Google::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(google, Google)
