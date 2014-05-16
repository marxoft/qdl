#include "mixturecloud.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

MixtureCloud::MixtureCloud(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp MixtureCloud::urlPattern() const {
    return QRegExp("http(s|)://(www.|)mixturecloud.com/media/download/[\\w-]+", Qt::CaseInsensitive);
}

bool MixtureCloud::urlSupported(const QUrl &url) const {
    return urlPattern().indexIn(url.toString()) == 0;
}

void MixtureCloud::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MixtureCloud::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http(s|)://\\w+.mixturecloud.com/down.php\\?d=[^\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QString fileName = re.cap().section('/', -1);
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void MixtureCloud::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void MixtureCloud::onWebPageDownloaded() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http(s|)://\\w+.mixturecloud.com/down.php\\?d=[^\"]+");

    if (re.indexIn(redirect) == 0) {
        QUrl url(redirect);
        emit downloadRequestReady(QNetworkRequest(url));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QUrl url(re.cap());
            emit downloadRequestReady(QNetworkRequest(url));
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

bool MixtureCloud::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(mixturecloud, MixtureCloud)
