#ifndef PUTLOCKER_H
#define PUTLOCKER_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class PutLocker : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit PutLocker(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new PutLocker; }
    inline QString iconName() const { return QString("putlocker.jpg"); }
    inline QString serviceName() const { return QString("PutLocker"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
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
    void getDownloadLink();
    void checkDownloadLink();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_hash;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // PUTLOCKER_H
