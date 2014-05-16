#include "megashares.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

MegaShares::MegaShares(QObject *parent) :
    RecaptchaPlugin(parent)
{
}

void MegaShares::getCaptcha(const QString &key) {
    this->setChallenge(key);
    QUrl url("http://d01.megashares.comindex.php");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("secgfx", "gfx");
    query.addQueryItem("random_num", key);
    url.setQuery(query);
#else
    url.addQueryItem("secgfx", "gfx");
    url.addQueryItem("random_num", key);
#endif
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MegaShares::onCaptchaDownloaded() {
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

void MegaShares::reportError(int errorCode) {
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

bool MegaShares::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(megasharesrecaptcha, MegaShares)
