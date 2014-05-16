#ifndef BAYFILES_H
#define BAYFILES_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class BayFiles : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit BayFiles(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new BayFiles; }
    inline QString iconName() const { return QString("bayfiles.jpg"); }
    inline QString serviceName() const { return QString("BayFiles"); }
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
    void getToken();

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkToken();
    void updateWaitTime();
    void onWaitFinished();
    void getDownloadLink();
    void checkDownloadLink();

signals:
    void currentOperationCancelled();

private:
    QString m_vfid;
    QString m_token;
    int m_wait;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // BAYFILES_H
