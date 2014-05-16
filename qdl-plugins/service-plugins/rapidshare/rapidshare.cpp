#include "rapidshare.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QTimer>
#include <QDateTime>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

RapidShare::RapidShare(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this))
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp RapidShare::urlPattern() const {
    return QRegExp("http(s|)://(www.|)rapidshare.com/(files|share)/\\w+", Qt::CaseInsensitive);
}

bool RapidShare::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void RapidShare::login(const QString &username, const QString &password) {
    QUrl url("https://api.rapidshare.com/cgi-bin/rsapi.cgi");
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("sub", "getaccountdetails");
    query.addQueryItem("login", username);
    query.addQueryItem("password", password);
    url.setQuery(query);
#else
    url.addQueryItem("sub", "getaccountdetails");
    url.addQueryItem("login", username);
    url.addQueryItem("password", password);
#endif
    QNetworkRequest request(url);
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void RapidShare::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    emit loggedIn(statusCode == 200);

    reply->deleteLater();
}

void RapidShare::checkUrl(const QUrl &webUrl) {
    m_type = webUrl.toString().contains("/files/") ? "files" : "share";
    QNetworkRequest request;

    if (m_type == "files") {
        request.setUrl(webUrl);
    }
    else {
        QString shareId = webUrl.toString().section('/', -1);
        QUrl url("https://api.rapidshare.com/cgi-bin/rsapi.cgi");
#if QT_VERSION >= 0x050000
        QUrlQuery query(url);
        query.addQueryItem("rsource", "web");
        query.addQueryItem("sub", "sharelinkcontent");
        query.addQueryItem("share", shareId);
        query.addQueryItem("cbid", "3");
        query.addQueryItem("cbf", "rsapi.system.jsonp.callback");
        query.addQueryItem("callt", QString::number(QDateTime::currentMSecsSinceEpoch()));
        url.setQuery(query);
#else
        url.addQueryItem("rsource", "web");
        url.addQueryItem("sub", "sharelinkcontent");
        url.addQueryItem("share", shareId);
        url.addQueryItem("cbid", "3");
        url.addQueryItem("cbf", "rsapi.system.jsonp.callback");
        url.addQueryItem("callt", QString::number(qint64(QDateTime::currentDateTime().toTime_t()) * 1000));
#endif
        request.setUrl(url);
    }

    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void RapidShare::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
        reply->deleteLater();
        this->checkUrl(redirect);
        return;
    }

    QString response(reply->readAll());

    if (m_type == "files") {
        QString errorString = response.section("ERROR:", 1, 1).section('(', 0, 0).trimmed();

        if ((!errorString.isEmpty()) && (!errorString.startsWith("File owner's public traffic exhausted"))) {
            emit urlChecked(false);
        }
        else {
            QString redirect = response.section("location=\"", -1).section('"', 0, 0);

            if (redirect.contains("/download/")) {
                QString fileName = reply->request().url().toString().section('/', -1);
                emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
            }
            else {
                emit urlChecked(false);
            }
        }
    }
    else {
        QStringList files = response.split("file:", QString::SkipEmptyParts);
        QString shareId = reply->request().url().queryItemValue("share");

        if ((files.isEmpty()) || (shareId.isEmpty())) {
            emit urlChecked(false);
        }
        else {
            files.removeFirst();

            while (!files.isEmpty()) {
                QStringList fileData = files.takeFirst().split(',', QString::SkipEmptyParts);
                
                if (fileData.size() >= 2) {
                    QString fileId = fileData.first();
                    QString fileName = fileData.at(1);
                    
                    if ((!fileId.isEmpty()) && (!fileName.isEmpty())) {
                        QUrl url("https://api.rapidshare.com/cgi-bin/rsapi.cgi");
#if QT_VERSION >= 0x050000
                        QUrlQuery query(url);
                        query.addQueryItem("share", shareId);
                        query.addQueryItem("rsource", "web");
                        query.addQueryItem("sub", "checkfiles");
                        query.addQueryItem("files", fileId);
                        query.addQueryItem("filenames", fileName);
                        query.addQueryItem("cbid", "5");
                        query.addQueryItem("cbf", "rsapi.system.jsonp.callback");
                        query.addQueryItem("callt", QString::number(QDateTime::currentMSecsSinceEpoch()));
                        url.setQuery(query);
#else
                        url.addQueryItem("share", shareId);
                        url.addQueryItem("rsource", "web");
                        url.addQueryItem("sub", "checkfiles");
                        url.addQueryItem("files", fileId);
                        url.addQueryItem("filenames", fileName);
                        url.addQueryItem("cbid", "5");
                        url.addQueryItem("cbf", "rsapi.system.jsonp.callback");
                        url.addQueryItem("callt", QString::number(qint64(QDateTime::currentDateTime().toTime_t()) * 1000));
#endif
                        emit urlChecked(true, url, this->serviceName(), fileName, files.isEmpty());
                    }
                    else if (files.isEmpty()) {
                        emit urlChecked(false);
                    }
                }
                else if (files.isEmpty()) {
                    emit urlChecked(false);
                }
            }
        }
    }

    reply->deleteLater();
}

void RapidShare::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);

    if (webUrl.toString().contains("#!download")) {
        this->constructDownloadUrl(webUrl.toString());
        return;
    }

    m_type = webUrl.toString().contains("/files/") ? "files" : "share";
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkDownloadUrl()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void RapidShare::checkDownloadUrl() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (!redirect.isEmpty()) {
        reply->deleteLater();
        this->getDownloadRequest(redirect);
        return;
    }

    QString response(reply->readAll());

    if (m_type == "files") {
        m_fileName = reply->request().url().toString().section('/', -1);

        if (response.startsWith("ERROR")) {
            QString errorString = response.section("ERROR:", 1, 1).section('(', 0, 0).trimmed();

            if (errorString.startsWith("File owner's public traffic exhausted")) {
                this->startWait(600000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
            }
            else if (errorString.startsWith("File not found")) {
                emit error(NotFound);
            }
            else {
                emit error(UnknownError);
            }
        }
        else {
            QString redirect = response.section("location=\"", -1).section('"', 0, 0);

            if (redirect.contains("/download/")) {
                this->constructDownloadUrl(redirect);
            }
            else {
                emit error(UnknownError);
            }
        }
    }
    else {
        QStringList fileData = response.section('\"', 1, 1).split(',', QString::SkipEmptyParts);
        QString shareId = reply->request().url().queryItemValue("share");
                
        if ((fileData.size() >= 6) && (!shareId.isEmpty())) {
            QString fileId = fileData.first();
            QString fileName = fileData.at(1);
            QString serverId = fileData.at(3) + fileData.at(5);
            QString bin = fileData.at(4);
                    
            if ((!fileId.isEmpty()) && (!fileName.isEmpty()) && (!serverId.isEmpty()) && (!bin.isEmpty())) {
                QUrl url(QString("https://rs%1.rapidshare.com/cgi-bin/rsapi.cgi").arg(serverId));
#if QT_VERSION >= 0x050000
                QUrlQuery query(url);
                query.addQueryItem("share", shareId);
                query.addQueryItem("sub", "download");
                query.addQueryItem("fileid", fileId);
                query.addQueryItem("filename", fileName);
                query.addQueryItem("bin", bin);
                url.setQuery(query);
#else
                url.addQueryItem("share", shareId);
                url.addQueryItem("sub", "download");
                url.addQueryItem("fileid", fileId);
                url.addQueryItem("filename", fileName);
                url.addQueryItem("bin", bin);
#endif
                emit downloadRequestReady(QNetworkRequest(url));
            }
            else {
                emit error(UnknownError);
            }
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

void RapidShare::constructDownloadUrl(const QString &url) {
    QStringList split = url.section("/download/", -1).split('/', QString::SkipEmptyParts);

    if (split.size() > 1) {
        QString server = split.takeFirst();
        QString fileId = split.takeFirst();
        QNetworkRequest request;
        request.setUrl(QUrl(QString("https://rs%1.rapidshare.com/cgi-bin/rsapi.cgi?sub=download&fileid=%2&filename=%3").arg(server).arg(fileId).arg(m_fileName)));
        emit downloadRequestReady(request);
    }
    else {
        emit error(UrlError);
    }
}

void RapidShare::startWait(int msecs) {
    if (msecs > 30000) {
        emit statusChanged(LongWait);
    }
    else {
        emit statusChanged(ShortWait);
    }

    emit waiting(msecs);
    m_waitTime = msecs;
    m_waitTimer->start(1000);
}

void RapidShare::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void RapidShare::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool RapidShare::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(rapidshare, RapidShare)
