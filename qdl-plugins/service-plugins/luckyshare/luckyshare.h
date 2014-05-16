#ifndef LUCKYSHARE_H
#define LUCKYSHARE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class LuckyShare : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit LuckyShare(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new LuckyShare; }
    inline QString iconName() const { return QString("luckyshare.jpg"); }
    inline QString serviceName() const { return QString("LuckyShare"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline QString errorString() const { return m_errorString; }
    inline int maximumConnections() const { return m_connections; }
    inline bool downloadsAreResumable() const { return m_connections != 1; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void startWait(int msecs);
    void getWaitTime(const QString &id);
    inline void setErrorString(const QString &errorString) { m_errorString = errorString; }

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void downloadCaptcha();
    void checkWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QString m_hash;
    QString m_captchaKey;
    QString m_errorString;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // LUCKYSHARE_H
