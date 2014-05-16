#include "cramit.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

Cramit::Cramit(QObject *parent) :
    RecaptchaPlugin(parent)
{
}

void Cramit::getCaptcha(const QString &key) {
    this->setChallenge(key);
    QUrl url(QString("http://main-2.cramit.in/captchas/%1.jpg").arg(key));
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Cramit::onCaptchaDownloaded() {
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

void Cramit::reportError(int errorCode) {
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

bool Cramit::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(cramitrecaptcha, Cramit)
