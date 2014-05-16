#ifndef RAPIDSHARE_H
#define RAPIDSHARE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class RapidShare : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit RapidShare(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new RapidShare; }
    inline QString iconName() const { return QString("rapidshare.jpg"); }
    inline QString serviceName() const { return QString("RapidShare"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private:
    void startWait(int msecs);
    void constructDownloadUrl(const QString &url);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void checkDownloadUrl();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QTimer *m_waitTimer;
    int m_waitTime;
    QString m_type;
    QString m_fileName;
};

#endif // RAPIDSHARE_H
