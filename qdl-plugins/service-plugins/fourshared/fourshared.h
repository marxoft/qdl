#ifndef FOURSHARED_H
#define FOURSHARED_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class FourShared : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit FourShared(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new FourShared; }
    inline QString iconName() const { return QString("fourshared.jpg"); }
    inline QString serviceName() const { return QString("4Shared"); }
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
    void getDownloadLimitInfo(const QString &fileId);
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkDownloadLimitInfo();
    void updateWaitTime();
    void onWaitFinished();
    void getDownloadLink();
    void checkDownloadLink();

signals:
    void currentOperationCancelled();

private:
    QUrl m_downloadUrl;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_wait;
    int m_connections;
};

#endif // FOURSHARED_H
