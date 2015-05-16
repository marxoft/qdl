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

#include "gofourup.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSettings>

QHash<QString, QString> GoFourUp::filehosts = QHash<QString, QString>();

GoFourUp::GoFourUp(QObject *parent) :
    ServicePlugin(parent)
{
    filehosts["Free"] = "18";
    filehosts["SolidFiles"] = "61";
    filehosts["HugeFiles"] = "56";
    filehosts["BillionUploads"] = "47";
    filehosts["180Upload"] = "45";
    filehosts["1fichier"] = "43";
    filehosts["Zippyshare"] = "42";
    filehosts["MegaShares"] = "32";
    filehosts["GameFront"] = "67";
    filehosts["SockShare"] = "59";
}

QRegExp GoFourUp::urlPattern() const {
    return QRegExp("http(s|)://(www.|)go4up.com/dl/\\w+", Qt::CaseInsensitive);
}

bool GoFourUp::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void GoFourUp::checkUrl(const QUrl &url) {
    m_filehost = QSettings("QDL", "QDL").value("Go4Up/filehost", "1fichier").toString();
    QUrl hostUrl(QString("http://go4up.com/rd/%1/%2").arg(url.path().section('/', 2, 2)).arg(filehosts.value(m_filehost, "43")));
    QNetworkRequest request(hostUrl);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void GoFourUp::checkUrlIsValid() {
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
        QRegExp re(QString("http(s|)://(www.|)([\\w\\.-_]+|)%1[^\"<]+").arg(m_filehost), Qt::CaseInsensitive);
        
        if (re.indexIn(response) >= 0) {
            QUrl url(re.cap());
            QString fileName = response.section("<title>Download", 1, 1).section("</title>", 0, 0).trimmed();

            if ((url.isValid()) && (!fileName.isEmpty())) {
                emit urlChecked(true, url, m_filehost, fileName);
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

void GoFourUp::getDownloadRequest(const QUrl &url) {
    Q_UNUSED(url)

    emit error(NotFound);
}

bool GoFourUp::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(gofourup, GoFourUp)
#endif
