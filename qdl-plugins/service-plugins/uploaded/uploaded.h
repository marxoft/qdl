#ifndef UPLOADED_H
#define UPLOADED_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Uploaded : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Uploaded(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Uploaded; }
    inline QString iconName() const { return QString("uploaded.jpg"); }
    inline QString serviceName() const { return QString("Uploaded"); }
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
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void downloadCaptcha();
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

#endif // UPLOADED_H
