#ifndef UPLOADABLE_H
#define UPLOADABLE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Uploadable : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Uploadable(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Uploadable; }
    inline QString iconName() const { return QString("uploadable.jpg"); }
    inline QString serviceName() const { return QString("Uploadable"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline int maximumConnections() const { return m_connections; }
    inline bool downloadsAreResumable() const { return m_connections != 1; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void getWaitTime();
    void getRedirect();
    void getDownloadLink();
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkWaitTime();
    void getDownloadCheck();
    void checkDownloadCheck();
    void onCaptchaSubmitted();
    void checkDownloadLink();
    void checkRedirect();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QString m_fileId;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // UPLOADABLE_H
