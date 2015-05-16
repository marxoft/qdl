/*
 * Copyright (C) 2014 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "recordedbabes.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

RecordedBabes::RecordedBabes(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp RecordedBabes::urlPattern() const {
    return QRegExp("http(s|)://(www\\.|)recordedbabes.com/.+", Qt::CaseInsensitive);
}

bool RecordedBabes::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void RecordedBabes::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
	request.setRawHeader("User-Agent", "Wget/1.13.4 (linux-gnu)");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void RecordedBabes::checkUrlIsValid() {
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
        QUrl videoUrl = response.section("embed src=\"", 1, 1).section('"', 0, 0);

        if (videoUrl.hasQueryItem("file")) {
            QString fileName = videoUrl.queryItemValue("file");
			
			if (!fileName.isEmpty()) {
				emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
			}
			else {
				emit urlChecked(false);
			}
        }
        else {
            emit urlChecked(false);
        }
    }

    reply->deleteLater();
}

void RecordedBabes::getDownloadRequest(const QUrl &url) {
    emit statusChanged(Connecting);
    QNetworkRequest request(url);
	request.setRawHeader("User-Agent", "Wget/1.13.4 (linux-gnu)");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(parseVideoPage()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void RecordedBabes::parseVideoPage() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QString response(reply->readAll());
    QUrl videoUrl = response.section("embed src=\"", 1, 1).section('"', 0, 0);

    if (videoUrl.hasQueryItem("file")) {
		QUrl url;
		url.setScheme(videoUrl.scheme());
		url.setAuthority(videoUrl.authority());
		url.setPath(videoUrl.path().section("/", 0, -2) + "/" + videoUrl.queryItemValue("file"));
		QNetworkRequest request(url);
		request.setRawHeader("User-Agent", "Wget/1.13.4 (linux-gnu)");
        emit downloadRequestReady(request);
    }
    else {
        emit error(UrlError);
    }

    reply->deleteLater();
}

bool RecordedBabes::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(recordedbabes, RecordedBabes)
#endif
