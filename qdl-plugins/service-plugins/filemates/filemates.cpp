#include "filemates.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QStringList>
#include <QRegExp>

FileMates::FileMates(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp FileMates::urlPattern() const {
    return QRegExp("http(s|)://(www.|)filemates.com/\\w+", Qt::CaseInsensitive);
}

bool FileMates::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void FileMates::login(const QString &username, const QString &password) {
    QString data = QString("op=login&login=%1&password=%2").arg(username).arg(password);
    QUrl url("http://filemates.com/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileMates::checkLogin() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch (statusCode) {
    case 302:
    case 200:
    case 201:
        m_connections = 0;
        emit loggedIn(true);
        break;
    default:
        m_connections = 1;
        emit loggedIn(false);
        break;
    }

    reply->deleteLater();
}

void FileMates::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileMates::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://files-dl\\d+\\.com/cgi-bin/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("fname\" value=\"", 1, 1).section('"', 0, 0);

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            QString urlString = reply->request().url().toString();

            if (!urlString.endsWith(fileName)) {
                if (!urlString.endsWith('/')) {
                    urlString.append('/');
                }

                urlString.append(fileName);
            }

            QUrl url(urlString);
            emit urlChecked(true, url, this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void FileMates::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_url = webUrl;
    QNetworkRequest request(m_url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileMates::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://files-dl\\d+\\.com/cgi-bin/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else if (!redirect.isEmpty()) {
        this->getDownloadRequest(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QNetworkRequest request;
            request.setUrl(QUrl(re.cap()));
            emit downloadRequestReady(request);
        }
        else {
            m_fileId = response.section("id\" value=\"", 1, 1).section('"', 0, 0);
            m_fileName = response.section("fname\" value=\"", 1, 1).section('"', 0, 0);

            if ((m_fileId.isEmpty()) || (m_fileName.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                this->getCaptcha();
            }
        }
    }

    reply->deleteLater();
}

void FileMates::getCaptcha() {
    QString data = QString("op=download1&id=%1&fname=%2&method_free=Free Download").arg(m_fileId).arg(m_fileName);
    QNetworkRequest request(m_url);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkCaptcha()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void FileMates::checkCaptcha() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://files-dl\\d+\\.com/cgi-bin/[^'\"]+");
    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (re.indexIn(redirect) == 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else if (!redirect.isEmpty()) {
        this->getDownloadRequest(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (re.indexIn(response) >= 0) {
            QNetworkRequest request;
            request.setUrl(QUrl(re.cap()));
            emit downloadRequestReady(request);
        }
        else {
            m_rand = response.section("rand\" value=\"", 1, 1).section('"', 0, 0);
            QString codeBlock = response.section("<td align=right>", 1, 1).section("</span></div>", 0, 0);
            QStringList codeList = codeBlock.split("padding-left:", QString::SkipEmptyParts);

            if (!codeList.isEmpty()) {
                codeList.removeFirst();

                QMap<int, QString> codeMap;

                int i = 1;

                while ((!codeList.isEmpty()) && (i < 5)) {
                    QString code = codeList.takeFirst();
                    int key = code.section('p', 0, 0).toInt();
                    QString value = QString::number(code.section(">&#", 1, 1).left(2).toInt() - 48);
                    codeMap[key] = value;
                    i++;
                }

                QList<int> codeKeys = codeMap.keys();
                qSort(codeKeys.begin(), codeKeys.end());

                foreach (int key, codeKeys) {
                    m_code.append(codeMap[key]);
                }
            }

            if ((m_rand.isEmpty()) || (m_code.isEmpty())) {
                QString errorString = response.section("<div class=\"err\">", 1, 1).section('<', 0, 0);

                if (!errorString.isEmpty()) {
                    if (errorString.startsWith("You have to wait")) {
                        int mins = errorString.section(" minutes", 0, 0).section(' ', -1).toInt();
                        int secs = errorString.section(" seconds", 0, 0).section(' ', -1).toInt();
                        this->startWait((mins * 60000) + (secs * 1000));
                        this->connect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
                    }
                    else {
                        emit error(UnknownError);
                    }
                }
                else {
                    emit error(UnknownError);
                }
            }
            else {
                this->startWait(30000);
                this->connect(this, SIGNAL(waitFinished()), this, SLOT(submitCaptcha()));
            }
        }
    }

    reply->deleteLater();
}

void FileMates::submitCaptcha() {
    QString data = QString("op=download2&id=%1&rand=%2&method_free=Free Download&code=%3&down_script=1").arg(m_fileId).arg(m_rand).arg(m_code);
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(submitCaptcha()));
}

void FileMates::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://files-dl\\d+\\.com/cgi-bin/[^'\"]+");
    QString response(reply->readAll());

    if (re.indexIn(response) >= 0) {
        QNetworkRequest request;
        request.setUrl(QUrl(re.cap()));
        emit downloadRequestReady(request);
    }
    else {
        emit error(UnknownError);
    }

    reply->deleteLater();
}

void FileMates::startWait(int msecs) {
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

void FileMates::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void FileMates::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool FileMates::cancelCurrentOperation() {
    m_waitTimer->stop();
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(filemates, FileMates)
