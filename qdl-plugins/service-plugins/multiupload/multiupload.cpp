#include "multiupload.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

Multiupload::Multiupload(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp Multiupload::urlPattern() const {
    return QRegExp("http(s|)://(www.|)multiupload.nl/\\w+", Qt::CaseInsensitive);
}

bool Multiupload::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Multiupload::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Multiupload::checkUrlIsValid() {
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
        
        QRegExp re("http(s|)://www\\d+.multiupload.nl:\\d+/files/[^\"]+");

        if (re.indexIn(response) >= 0) {
            QString fileName = re.cap().section('/', -1);
        
            if (fileName.isEmpty()) {
                emit urlChecked(false);
            }
            else {
                emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
            }
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void Multiupload::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkWebPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Multiupload::checkWebPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
        this->getDownloadRequest(redirect);
    }
    else {
        QString response(reply->readAll());
        
        QRegExp re("http(s|)://www\\d+.multiupload.nl:\\d+/files/[^\"]+");

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

bool Multiupload::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(multiupload, Multiupload)
