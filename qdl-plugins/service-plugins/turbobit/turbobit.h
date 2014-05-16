#ifndef TURBOBIT_H
#define TURBOBIT_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class TurboBit : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit TurboBit(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new TurboBit; }
    inline QString iconName() const { return QString("turbobit.jpg"); }
    inline QString serviceName() const { return QString("TurboBit"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaLink; }
    inline QString recaptchaServiceName() const { return QString("TurboBit"); }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void getCaptcha();
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkCaptcha();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void downloadCaptcha();
    void onWaitFinished();
    void getDownloadLink();
    void checkDownloadLink();

signals:
    void currentOperationCancelled();

private:
    QString m_fileId;
    QString m_captchaLink;
    QString m_captchaType;
    QString m_captchaSubtype;
    QTimer *m_waitTimer;
    int m_waitTime;
};

#endif // TURBOBIT_H
