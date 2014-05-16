#ifndef ZIPPYSHARE_H
#define ZIPPYSHARE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Zippyshare : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Zippyshare(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Zippyshare; }
    inline QString iconName() const { return QString("zippyshare.jpg"); }
    inline QString serviceName() const { return QString("Zippyshare"); }
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
    QString m_url;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_wait;
    int m_connections;
};

#endif // ZIPPYSHARE_H
