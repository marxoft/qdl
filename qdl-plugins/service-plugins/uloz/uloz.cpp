#include "uloz.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QDateTime>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

using namespace QtJson;

Uloz::Uloz(QObject *parent) :
    ServicePlugin(parent),
    m_connections(1)
{
}

QRegExp Uloz::urlPattern() const {
    return QRegExp("http(s|)://(www.|)(uloz.to|ulozto.net)/\\w+/[-\\w]+", Qt::CaseInsensitive);
}

bool Uloz::urlSupported(const QUrl &url) const {
    return this->urlPattern().indexIn(url.toString()) == 0;
}

void Uloz::login(const QString &username, const QString &password) {
    QString data = QString("username=%1&password=%2").arg(username).arg(password);
    QUrl url("http://uloz.to/login?do=loginForm-submit");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uloz::checkLogin() {
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

void Uloz::checkUrl(const QUrl &webUrl) {
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(checkUrlIsValid()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uloz::checkUrlIsValid() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit urlChecked(false);
        return;
    }

    QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
    QRegExp re("http://\\w+.uloz.to/Ps;Hs;fid=\\d+[^'\"]+");

    if ((!redirect.isEmpty()) && (re.indexIn(redirect) == -1)) {
        this->checkUrl(QUrl(redirect));
    }
    else {
        QString response(reply->readAll());
        QString fileName = response.section("<title>", 1).section(" | Ulo", 0, 0);

        if (fileName.isEmpty()) {
            emit urlChecked(false);
        }
        else {
            emit urlChecked(true, reply->request().url(), this->serviceName(), fileName);
        }
    }

    reply->deleteLater();
}

void Uloz::getDownloadRequest(const QUrl &webUrl) {
    emit statusChanged(Connecting);
    m_url = webUrl;
    QNetworkRequest request(webUrl);
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    QNetworkReply *reply = this->networkAccessManager()->get(request);
    this->connect(reply, SIGNAL(finished()), this, SLOT(onWebPageDownloaded()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uloz::onWebPageDownloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QRegExp re("http://\\w+.uloz.to/Ps;Hs;fid=\\d+[^'\"]+");
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
	        m_token = response.section("frmfreeDownloadForm-_token_\" value=\"", 1, 1).section('"', 0, 0);
            m_timestamp = response.section("frmfreeDownloadForm-ts\" value=\"", 1, 1).section('"', 0, 0);
            m_cid = response.section("frmfreeDownloadForm-cid\" value=\"", 1, 1).section('"', 0, 0);
            m_sign = response.section("frmfreeDownloadForm-sign\" value=\"", 1, 1).section('"', 0, 0);

            if ((m_token.isEmpty()) || (m_timestamp.isEmpty()) || (m_cid.isEmpty()) || (m_sign.isEmpty())) {
                emit error(UnknownError);
            }
            else {
                m_captchaKey = QString::number(QDateTime::currentMSecsSinceEpoch());
                emit statusChanged(CaptchaRequired);
            }
        }
    }

    reply->deleteLater();
}

void Uloz::submitCaptchaResponse(const QString &challenge, const QString &response) {
    QString data = QString("captcha_value=%1&%2&_token_=%3&ts=%4&cid=%5&sign=%6").arg(response).arg(challenge).arg(m_token).arg(m_timestamp).arg(m_cid).arg(m_sign);
    QUrl url(m_url);
#if QT_VERSION >= 0x050000
    QUrlQuery query(url);
    query.addQueryItem("do", "downloadDialog-freeDownloadForm-submit");
    url.setQuery(query);
#else
    url.addQueryItem("do", "downloadDialog-freeDownloadForm-submit");
#endif
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Accept", "text/javascript, text/html, application/xml, text/xml, */*");
    request.setRawHeader("Accept-Language", "en-GB,en-US;q=0.8,en;q=0.6");
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = this->networkAccessManager()->post(request, data.toUtf8());
    this->connect(reply, SIGNAL(finished()), this, SLOT(onCaptchaSubmitted()));
    this->connect(this, SIGNAL(currentOperationCancelled()), reply, SLOT(deleteLater()));
}

void Uloz::onCaptchaSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(this->sender());

    if (!reply) {
        emit error(NetworkError);
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

    if (redirect.isValid()) {
        emit downloadRequestReady(QNetworkRequest(redirect));
    }
    else {
        QString response(reply->readAll());
        QVariantMap result = Json::parse(response).toMap();

        if (!result.isEmpty()) {
            redirect = result.value("url").toUrl();

            if (redirect.isValid()) {
                emit downloadRequestReady(QNetworkRequest(redirect));
            }
            else {
                QVariantList errors = result.value("errors").toList();

                if (!errors.isEmpty()) {
                    QString errorString = errors.first().toString();

                    if ((errorString.startsWith("Error rewriting")) || (errorString.startsWith("Text je op"))) {
                        m_captchaKey = QString::number(QDateTime::currentMSecsSinceEpoch());
                        emit error(CaptchaError);
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
        else if ((response.contains("Error rewriting")) || (response.contains("Text je ops"))) {
            m_captchaKey = QString::number(QDateTime::currentMSecsSinceEpoch());
            emit error(CaptchaError);
        }
        else {
            emit error(UnknownError);
        }
    }

    reply->deleteLater();
}

bool Uloz::cancelCurrentOperation() {
    emit currentOperationCancelled();

    return true;
}

Q_EXPORT_PLUGIN2(uloz, Uloz)
