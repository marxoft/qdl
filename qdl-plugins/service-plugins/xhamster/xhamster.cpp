#include "xhamster.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

XHamster::XHamster(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp XHamster::urlPattern() const {
    return QRegExp("http(s|)://(www.|)xhamster.com/movies/\\d+", Qt::CaseInsensitive);
}

bool XHamster::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void XHamster::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void XHamster::checkUrlIsValid() {
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
        QString varString = QByteArray::fromPercentEncoding(response.section("flashvars\" value=\"", 1, 1).section('"', 0, 0).toUtf8());

        if (!varString.isEmpty()) {
            QString fileName = QByteArray::fromPercentEncoding(varString.section("title=", 1, 1).section('&', 0, 0).toUtf8()).replace('+', ' ') + ".flv";
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void XHamster::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void XHamster::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QUrl url(response.section("file=\"", 1, 1).section('"', 0, 0));

    if (url.isEmpty()) {
        QString varString = response.section("flashvars\" value=\"", 1, 1).section('"', 0, 0);

        if (!varString.isEmpty()) {
	        url = QUrl::fromEncoded("http://xhamster.com?" + varString.toUtf8());
	        QString srv = url.queryItemValue("srv");
	        QString file = url.queryItemValue("file");

            if (file.startsWith("http")) {
                QNetworkRequest request;
                request.setUrl(QUrl(file));
                emit downloadRequestReady(request);
            }
            else {
                QNetworkRequest request;
                request.setUrl(QUrl::fromEncoded(QString("%1/key=%2").arg(srv, file).toUtf8()));
                emit downloadRequestReady(request);
            }
        }
        else {
            emit error(UrlError);
        }
    }
    else {
        emit downloadRequestReady(QNetworkRequest(url));
    }

    reply->deleteLater();
}

bool XHamster::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(xhamster, XHamster)
