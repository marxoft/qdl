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

#include "videograbber.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>
#include <QStringList>

VideoGrabber::VideoGrabber(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp VideoGrabber::urlPattern() const {
    return QRegExp("no_url_pattern");
}

bool VideoGrabber::urlSupported(const QUrl &url) const {
    Q_UNUSED(url)

    return false;
}

void VideoGrabber::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void VideoGrabber::checkUrlIsValid() {
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
        QStringList urls;
        const int limit = QSettings("QDL", "QDL").value("VideoGrabber/videoLimit", 0).toInt();
        QRegExp re("http(s|)://[^'\"<\\s]+\\.(mp4|flv|avi|divx|mpg|mpeg|mpeg2|mpeg4|ts|mkv|wmv|xvid|mov)(?=['\"<])", Qt::CaseInsensitive);
        int pos = 0;

        while (((pos = re.indexIn(response, pos)) != -1) && ((limit == 0) || (urls.size() <= limit))) {
            urls.append(re.cap(0));
            pos += re.matchedLength();
        }

        if (urls.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            urls.removeDuplicates();

            while (!urls.isEmpty()) {
                QString url = urls.takeFirst();
                emit urlChecked(true, QUrl(url), this->serviceName(), url.mid(url.lastIndexOf('/') + 1), urls.isEmpty());
            }
        }
    }

    reply->deleteLater();
}

void VideoGrabber::getDownloadRequest(const QUrl &url) {
    emit downloadRequestReady(QNetworkRequest(url));
}

bool VideoGrabber::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(videograbber, VideoGrabber)
