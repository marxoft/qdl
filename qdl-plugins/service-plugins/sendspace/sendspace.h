#ifndef SENDSPACE_H
#define SENDSPACE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class SendSpace : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit SendSpace(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new SendSpace; }
    inline QString iconName() const { return QString("sendspace.jpg"); }
    inline QString serviceName() const { return QString("SendSpace"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

private:
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_wait;
    int m_connections;
};

#endif // SENDSPACE_H
