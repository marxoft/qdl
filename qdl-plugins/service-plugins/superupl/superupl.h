#ifndef SUPERUPL_H
#define SUPERUPL_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class SuperUpl : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit SuperUpl(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new SuperUpl; }
    inline QString iconName() const { return QString("superupl.jpg"); }
    inline QString serviceName() const { return QString("SuperUpl"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void getWaitTime();
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkWaitTime();
    void downloadCaptcha();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_fileId;
    QString m_fileName;
    QString m_rand;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // SUPERUPL_H
