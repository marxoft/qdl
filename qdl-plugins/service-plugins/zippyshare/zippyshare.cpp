#include "zippyshare.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QTimer>

Zippyshare::Zippyshare(QObject *parent) :
    ServicePlugin(parent),
    m_waitTimer(new QTimer(this)),
    m_waitTime(0),
    m_wait(0),
    m_connections(1)
{
    this->connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(updateWaitTime()));
}

QRegExp Zippyshare::urlPattern() const {
    return QRegExp("http://www\\d+.zippyshare.com/v/\\d+", Qt::CaseInsensitive);
}

bool Zippyshare::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Zippyshare::login(const QString &username, const QString &password) {
    QString data = QString("login=%1&pass=%2").arg(username).arg(password);
    QUrl url("http://www.zippyshare.com/services/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Zippyshare::checkLogin() {
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

void Zippyshare::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Zippyshare::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://www\\d+.zippyshare.com/d/\\d+/[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());

        if (response.contains("does not exist")) {
            emit urlChecked(false);
        }
        else {
            QString fileName = response.section("og:title\" content=\"", 1, 1).section('"', 0, 0).trimmed();

            if (fileName.isEmpty()) {
                emit urlChecked(false);
            }
            else {
                emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
            }
        }
    }

    reply->deleteLater();
}

void Zippyshare::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_url = webUrl.toString();
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Zippyshare::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://www\\d+.zippyshare.com/d/\\d+/[^'\"]+");
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
        else if (response.contains("does not exist")) {
            emit error(NotFound);
        }
        else {
            QString varA = response.section("var a = ", 1, 1).section(';', 0, 0);
            QString varB = response.section("var b = ", 1, 1).section(';', 0, 0);
            QString varC = response.section("var c = ", 1, 1).section(';', 0, 0);
            QString varD = response.section("var d = ", -1).section(';', 0, 0);

            int a = 0;
            int b = 0;
            int c = 0;
            int d = 0;

            if (varA.contains('%')) {
                a = varA.section('%', 0, 0).toInt() % varA.section('%', -1).toInt();
            }
            else {
                a = varA.toInt();
            }

            if (varB.contains('%')) {
                b = varB.section('%', 0, 0).toInt() % varB.section('%', -1).toInt();
            }
            else {
                b = varB.toInt();
            }

            if (varC.contains('%')) {
                c = varC.section('%', 0, 0).toInt() % varC.section('%', -1).toInt();
            }
            else {
                c = varC.toInt();
            }

            if (varD.contains('%')) {
                d = varD.section('%', 0, 0).toInt() % varD.section('%', -1).toInt();
            }
            else {
                d = varD.toInt();
            }

            int num = a * b + c + d;

            if (num > 0) {
                QString urlPartOne = m_url.section("/v/", 0, 0);
                QString urlPartTwo = response.section("getElementById('dlbutton').href = \"", 1, 1).section('"', 0, 0);
                QString urlPartThree = QString::number(num);
                QString urlPartFour = response.section("(a * b + c + d)+\"", 1, 1).section('"', 0, 0);
                QUrl url(urlPartOne + urlPartTwo + urlPartThree + urlPartFour);

                if (url.isValid()) {
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
    }

    reply->deleteLater();
}

void Zippyshare::startWait(int msecs) {
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

void Zippyshare::updateWaitTime() {
    m_waitTime -= m_waitTimer->interval();
    emit waiting(m_waitTime);

    if (m_waitTime <= 0) {
        m_waitTimer->stop();
        emit waitFinished();
    }
}

void Zippyshare::onWaitFinished() {
    emit statusChanged(Ready);
    this->disconnect(this, SIGNAL(waitFinished()), this, SLOT(onWaitFinished()));
}

bool Zippyshare::cancelCurrentOperation() {
    m_waitTimer->stop();
    this->disconnect(this, SIGNAL(waitFinished()), this, 0);
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(zippyshare, Zippyshare)
