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

#include "imagegrabber.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QStringList>

static const QStringList IMAGE_HOSTS = QStringList() << "imgserve.net"
                                                     << "imgtiger.com";

ImageGrabber::ImageGrabber(QObject *parent) :
    ServicePlugin(parent)
{
}

QRegExp ImageGrabber::urlPattern() const {
    return QRegExp("no_url_pattern");
}

bool ImageGrabber::urlSupported(const QUrl &url) const {
    Q_UNUSED(url)

    return false;
}

void ImageGrabber::checkUrl(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void ImageGrabber::checkUrlIsValid() {
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
        QRegExp re(QString("http(s|)://(www.|)[^'\"<\\s\\[\\]]+(%1)[^'\"<\\s\\[\\]]+\\.(jpg|jpeg|png|gif|bmp)(?=[^\\w])").arg(IMAGE_HOSTS.join("|")), Qt::CaseInsensitive);
        int pos = 0;

        while ((pos = re.indexIn(response, pos)) != -1) {
            urls.append(re.cap(0).replace("small", "big").remove("_thumb"));
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

void ImageGrabber::getDownloadRequest(const QUrl &url) {
    emit downloadRequestReady(QNetworkRequest(url));
}

bool ImageGrabber::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(imagegrabber, ImageGrabber)
#endif
