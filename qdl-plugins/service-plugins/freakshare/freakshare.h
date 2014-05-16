#ifndef FREAKSHARE_H
#define FREAKSHARE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class FreakShare : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit FreakShare(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new FreakShare; }
    inline QString iconName() const { return QString("freakshare.jpg"); }
    inline QString serviceName() const { return QString("FreakShare"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return QString("6Lftl70SAAAAAItWJueKIVvyG0QfLgmAgzKgTbDT"); }
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
    QString m_did;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // FREAKSHARE_H
