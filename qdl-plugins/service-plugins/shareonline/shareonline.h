#ifndef SHAREONLINE_H
#define SHAREONLINE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class ShareOnline : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit ShareOnline(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new ShareOnline; }
    inline QString iconName() const { return QString("shareonline.jpg"); }
    inline QString serviceName() const { return QString("ShareOnline"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return "6LdatrsSAAAAAHZrB70txiV5p-8Iv8BtVxlTtjKX"; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void getCaptchaPage();
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkCaptchaPage();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QString m_fileId;
    QString m_captchaId;

    QUrl m_captchaUrl;
    QUrl m_downloadUrl;

    QTimer *m_waitTimer;

    int m_waitTime;
    int m_dlWaitTime;
    int m_connections;
};

#endif // SHAREONLINE_H
