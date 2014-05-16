#include "vimeo.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>

using namespace QtJson;

Vimeo::Vimeo(QObject *parent) :
    ServicePlugin(parent)
{
    m_formatList << "hd" << "sd";
}

QRegExp Vimeo::urlPattern() const {
    return QRegExp("http(s|)://vimeo.com/\\d+", Qt::CaseInsensitive);
}

bool Vimeo::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Vimeo::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Vimeo::checkUrlIsValid() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString response(reply->readAll());
    QString params = response.section("\"h264\":", 1, 1).section(",\"hls\":", 0, 0);
    QVariantMap formatMap = Json::parse(params).toMap();

    if (formatMap.isEmpty()) {
        emit error(UnknownError);
    }
    else {
        QVariantMap videoMap = formatMap.value("video").toMap();
        QString fileName = videoMap.value("title").toString() + ".mp4";
        emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
    }

    reply->deleteLater();
}

void Vimeo::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    QNetworkRequest request(webUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Vimeo::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QString params = response.section("\"h264\":", 1, 1).section(",\"hls\":", 0, 0);
    QVariantMap formatMap = Json::parse(params).toMap();

    if (formatMap.isEmpty()) {
        emit error(UnknownError);
    }
    else {
        QUrl url;
        int i = m_formatList.indexOf(QSettings("QDL", "QDL").value("Vimeo/videoFormat", "sd").toString());

        while ((url.isEmpty()) && (i  < m_formatList.size())) {
            url.setUrl(formatMap.value(m_formatList.at(i), "").toMap().value("url").toString());
            i++;
        }

        if (url.isEmpty()) {
            emit error(UnknownError);
        }
        else {
            emit downloadRequestReady(QNetworkRequest(url));
        }
    }

    reply->deleteLater();
}

bool Vimeo::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(vimeo, Vimeo)
