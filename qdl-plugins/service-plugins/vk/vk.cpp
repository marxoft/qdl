#include "vk.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>
#include <QStringList>

using namespace QtJson;

static const QStringList FORMATS = QStringList() << "url1080" << "url720" << "url480" << "url360" << "url240";

Vk::Vk(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp Vk::urlPattern() const {
    return QRegExp("http(s|)://(m\\.|)vk.com/video.+", Qt::CaseInsensitive);
}

bool Vk::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Vk::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Vk::checkUrlIsValid() {
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
        QVariantMap params = Json::parse(response.section("var vars =", 1, 1).section(QRegExp("[\n\r]"), 0, 0).trimmed()).toMap();
        QString fileName = params.value("md_title").toString();

        if (!fileName.isEmpty()) {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName + ".mp4");
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void Vk::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Vk::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QVariantMap params = Json::parse(response.section("var vars =", 1, 1).section(QRegExp("[\n\r]"), 0, 0).trimmed()).toMap();

    if (!params.isEmpty()) {
        QString format = QSettings("QDL", "QDL").value("VK/videoFormat", "url360").toString();
        QUrl url;
        int i = qMax(0, FORMATS.indexOf(format));

        while ((url.isEmpty()) && (i < FORMATS.size())) {
            url.setUrl(params.value(FORMATS.at(i)).toString());
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

bool Vk::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(vk, Vk)