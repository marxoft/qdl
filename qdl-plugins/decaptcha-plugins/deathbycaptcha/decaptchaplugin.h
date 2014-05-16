#ifndef DECAPTCHAPLUGIN_H
#define DECAPTCHAPLUGIN_H

#include <QObject>
#include "decaptchainterface.h"

class QNetworkAccessManager;
class QUrl;
class QString;

class DecaptchaPlugin : public QObject, public DecaptchaInterface
{
    Q_OBJECT
    Q_INTERFACES(DecaptchaInterface)

public:
    enum ErrorType {
        CaptchaNotFound,
        CaptchaUnsolved,
        ServiceUnavailable,
        Unauthorised,
        BadRequest,
        InternalError,
        NetworkError,
        UnknownError
    };

public:
    explicit DecaptchaPlugin(QObject *parent = 0) : QObject(parent), m_nam(0) {}
    virtual ~DecaptchaPlugin() {}
    inline DecaptchaPlugin* getDecaptchaPlugin() { return this; }
    virtual DecaptchaPlugin* createDecaptchaPlugin() = 0;
    inline QNetworkAccessManager *networkAccessManager() const { return m_nam; }
    inline void setNetworkAccessManager(QNetworkAccessManager *manager) { m_nam = manager; }
    inline QString username() const { return m_username; }
    inline QString password() const { return m_password; }
    virtual QString captchaId() const { return QString(); }
    virtual QString iconName() const = 0;
    virtual QString serviceName() const = 0;
    virtual void getCaptchaResponse(const QByteArray &data) = 0;
    virtual bool cancelCurrentOperation() = 0;
    virtual void reportIncorrectCaptchaResponse(const QString &id) { Q_UNUSED(id) }

public slots:
    inline virtual void login(const QString &username, const QString &password) { m_username = username; m_password = password; emit credentialsChanged(username, password); emit loggedIn((!username.isEmpty()) && (!password.isEmpty()));}

signals:
    void captchaResponseReady(const QString &response);
    void credentialsChanged(const QString &username, const QString &password);
    void loggedIn(bool ok);
    void error(DecaptchaPlugin::ErrorType errorType);

protected:
    QNetworkAccessManager *m_nam;
    QString m_username;
    QString m_password;
};

#endif // DECAPTCHAPLUGIN_H
