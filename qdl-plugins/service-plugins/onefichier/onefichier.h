#ifndef ONEFICHIER_H
#define ONEFICHIER_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class OneFichier : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit OneFichier(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new OneFichier; }
    inline QString iconName() const { return QString("onefichier.jpg"); }
    inline QString serviceName() const { return QString("1fichier"); }
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
    void getDownloadLink(const QUrl &url);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkToken();
    void updateWaitTime();
    void onWaitFinished();
    void checkDownloadLink();

signals:
    void currentOperationCancelled();

private:
    int m_wait;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // ONEFICHIER_H
