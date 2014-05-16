#ifndef UPSTORE_H
#define UPSTORE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Upstore : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Upstore(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Upstore; }
    inline QString iconName() const { return QString("upstore.jpg"); }
    inline QString serviceName() const { return QString("Upstore"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void downloadCaptcha();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // UPSTORE_H
