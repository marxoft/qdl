#ifndef GIGAPETA_H
#define GIGAPETA_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class GigaPeta : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit GigaPeta(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new GigaPeta; }
    inline QString iconName() const { return QString("gigapeta.jpg"); }
    inline QString serviceName() const { return QString("GigaPeta"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return QString::number(qrand()); }
    inline QString recaptchaServiceName() const { return QString("GigaPeta"); }
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
    void onWaitFinished();
    void downloadCaptcha();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // GIGAPETA_H