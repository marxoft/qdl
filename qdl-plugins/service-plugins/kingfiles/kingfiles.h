#ifndef KINGFILES_H
#define KINGFILES_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Kingfiles : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Kingfiles(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Kingfiles; }
    inline QString iconName() const { return QString("kingfiles.jpg"); }
    inline QString serviceName() const { return QString("Kingfiles"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline QString recaptchaServiceName() const { return QString("Kingfiles"); }
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
    void onCaptchaSubmitted();
    void updateWaitTime();
    void onWaitFinished();
    void downloadCaptcha();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_fileId;
    QString m_fileName;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // KINGFILES_H
