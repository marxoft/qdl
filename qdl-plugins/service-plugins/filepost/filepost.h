#ifndef FILEPOST_H
#define FILEPOST_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class FilePost : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit FilePost(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new FilePost; }
    inline QString iconName() const { return QString("filepost.jpg"); }
    inline QString serviceName() const { return QString("FilePost"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &webUrl);
    void getDownloadRequest(const QUrl &webUrl);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void startWait(int msecs);
    void getWaitTime();

private slots:
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void downloadCaptcha();
    void checkWaitTime();
    void onWaitFinished();
    void checkLogin();

signals:
    void currentOperationCancelled();

private:
    QString m_code;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // FILEPOST_H
