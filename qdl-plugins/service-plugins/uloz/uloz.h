#ifndef ULOZ_H
#define ULOZ_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class Uloz : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Uloz(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Uloz; }
    inline QString iconName() const { return QString("uloz.jpg"); }
    inline QString serviceName() const { return QString("Uloz"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaId; }
    inline QString recaptchaServiceName() const { return QString("Uloz"); }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_captchaId;
    QString m_captchaKey;
    QString m_timestamp;
    QString m_cid;
    QString m_sign;
    int m_connections;
};

#endif // ULOZ_H
