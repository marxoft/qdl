#ifndef SERVICEPLUGIN_H
#define SERVICEPLUGIN_H

#include <QObject>
#include <QUrl>
#include "serviceinterface.h"

class QNetworkAccessManager;
class QNetworkRequest;
class QString;
class QRegExp;

class ServicePlugin : public QObject, public ServiceInterface
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    enum Status {
        Connecting,
        ShortWait,
        LongWait,
        CaptchaRequired,
        Ready
    };

    enum ErrorType {
        LoginError,
        UrlError,
        CaptchaError,
        Unauthorised,
        BadRequest,
        NotFound,
        TrafficExceeded,
        ServiceUnavailable,
        NetworkError,
        UnknownError
    };

public:
    explicit ServicePlugin(QObject *parent = 0) : QObject(parent), m_nam(0) {}
    virtual ~ServicePlugin() {}
    inline ServicePlugin* getServicePlugin() { return this; }
    virtual ServicePlugin* createServicePlugin() = 0;
    inline QNetworkAccessManager *networkAccessManager() const { return m_nam; }
    inline void setNetworkAccessManager(QNetworkAccessManager *manager) { m_nam = manager; }
    virtual void login(const QString &username, const QString &password) { Q_UNUSED(username) Q_UNUSED(password) }
    virtual QString iconName() const = 0;
    virtual QString serviceName() const = 0;
    virtual QRegExp urlPattern() const = 0;
    virtual bool urlSupported(const QUrl &url) const = 0;
    virtual void checkUrl(const QUrl &url) = 0;
    virtual void getDownloadRequest(const QUrl &url) = 0;
    virtual bool loginSupported() const = 0;
    virtual bool recaptchaRequired() const = 0;
    virtual bool cancelCurrentOperation() = 0;
    inline virtual int maximumConnections() const { return 1; } // < 1 means unlimited
    inline virtual bool downloadsAreResumable() const { return true; }
    inline virtual QString recaptchaServiceName() const { return QString("Google"); }
    inline virtual QString recaptchaKey() const { return QString(); }
    inline virtual QString errorString() const { return QString(); }

public slots:
    inline virtual void submitCaptchaResponse(const QString &challenge, const QString &response) { Q_UNUSED(challenge) Q_UNUSED(response) }

signals:
    void urlChecked(bool ok, const QUrl &url = QUrl(), const QString &service = QString(), const QString &fileName = QString(), bool done = true);
    void downloadRequestReady(const QNetworkRequest &request, QByteArray data = QByteArray());
    void waiting(int msecs);
    void error(ServicePlugin::ErrorType errorType);
    void waitFinished();
    void statusChanged(ServicePlugin::Status status);
    void loggedIn(bool ok);

protected:
    QNetworkAccessManager *m_nam;
};

#endif // SERVICEPLUGIN_H
