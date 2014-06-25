#ifndef KEEPTOSHARE_H
#define KEEPTOSHARE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class KeepToShare : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit KeepToShare(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new KeepToShare; }
    inline QString iconName() const { return QString("keeptoshare.jpg"); }
    inline QString serviceName() const { return QString("Keep2Share"); }
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
    void loginK2S(const QString &username, const QString &password);
    void loginKeep2s(const QString &username, const QString &password);
    void getCaptchaKey();
    void startWait(int msecs);

private slots:
    void checkKeep2ShareLogin();
    void checkK2SLogin();
    void checkKeep2sLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkCaptchaKey();
    void onCaptchaSubmitted();
    void getDownloadRequest();
    void checkDownloadRequest();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QString m_user;
    QString m_pass;
    QUrl m_url;
    QString m_fileId;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // KEEPTOSHARE_H
