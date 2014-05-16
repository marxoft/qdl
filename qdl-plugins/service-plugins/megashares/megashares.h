#ifndef MEGASHARES_H
#define MEGASHARES_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class MegaShares : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit MegaShares(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new MegaShares; }
    inline QString iconName() const { return QString("megashares.jpg"); }
    inline QString serviceName() const { return QString("MegaShares"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline QString recaptchaServiceName() const { return QString("MegaShares"); }
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

signals:
    void currentOperationCancelled();

private:
    QString m_fileName;
    QUrl m_url;
    QUrl m_downloadUrl;
    QString m_captchaKey;
    QString m_passportKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // MEGASHARES_H
