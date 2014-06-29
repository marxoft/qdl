#include "porntube.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>
#include <QStringList>

using namespace QtJson;

static const QStringList FORMATS = QStringList() << "1080" << "720" << "480" << "360" << "240";

PornTube::PornTube(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp PornTube::urlPattern() const {
    return QRegExp("http://(www\\.|)porntube.com/videos/.+", Qt::CaseInsensitive);
}

bool PornTube::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void PornTube::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void PornTube::checkUrlIsValid() {
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
        QString fileName = response.section("itemtype=\"http://schema.org/VideoObject\"><meta itemprop=\"name\" content=\"", 1, 1).section('"', 0, 0).trimmed();

        if (!fileName.isEmpty()) {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName + ".mp4");
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void PornTube::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void PornTube::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString id = response.section("icon-cloud-download\"></i><button data-id=\"", 1, 1).section('"', 0, 0).trimmed();

    if (!id.isEmpty()) {
        this->getVideoParams(id);
    }
    else {
        emit error(UrlError);
    }

    reply->deleteLater();
}

void PornTube::getVideoParams(const QString &id) {
    QUrl url(QString("http://tkn.porntube.com/%1/desktop/%2").arg(id).arg(FORMATS.join("+")));
    QNetworkRequest request(url);
    request.setRawHeader("ORIGIN", "http://www.porntube.com");
    QNetworkReply *reply = this->networkAccessManager()->post(request, "");
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoParams()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void PornTube::parseVideoParams() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap params = Json::parse(response).toMap();

    if (!params.isEmpty()) {
        QString format = QSettings("QDL", "QDL").value("PornTube/videoFormat", "360").toString();
        QUrl url;
        int i = qMax(0, FORMATS.indexOf(format));

        while ((url.isEmpty()) && (i < FORMATS.size())) {
            url.setUrl(params.value(FORMATS.at(i)).toMap().value("token").toString());
            i++;
        }

        if (!url.isEmpty()) {
            emit downloadRequestReady(QNetworkRequest(url));
        }
        else {
            emit error(UrlError);
        }
    }
    else {
        emit error(UrlError);
    }

    reply->deleteLater();
}

bool PornTube::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(porntube, PornTube)
