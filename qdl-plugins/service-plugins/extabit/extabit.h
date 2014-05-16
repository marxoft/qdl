#ifndef EXTABIT_H
#define EXTABIT_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Extabit : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Extabit(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Extabit; }
    inline QString iconName() const { return QString("extabit.jpg"); }
    inline QString serviceName() const { return QString("Extabit"); }
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
    void getDownloadPage(const QUrl &url);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void downloadCaptcha();
    void checkDownloadPage();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_id;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // EXTABIT_H
