#ifndef NETLOAD_H
#define NETLOAD_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Netload : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Netload(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Netload; }
    inline QString iconName() const { return QString("netload.jpg"); }
    inline QString serviceName() const { return QString("Netload"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline QString recaptchaServiceName() const { return QString("Netload"); }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void getWaitTime();
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkWaitTime();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void downloadCaptcha();
    void onWaitFinished();
    void getDownloadLink();
    void checkDownloadLink();

signals:
    void currentOperationCancelled();

private:
    QString m_id;
    QString m_fileId;
    QUrl m_waitUrl;
    QUrl m_downloadUrl;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // NETLOAD_H
