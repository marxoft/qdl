#ifndef FILEVICE_H
#define FILEVICE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class FileVice : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit FileVice(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new FileVice; }
    inline QString iconName() const { return QString("filevice.jpg"); }
    inline QString serviceName() const { return QString("FileVice"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline QString recaptchaServiceName() const { return QString("FileVice"); }
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
    QString m_fileId;
    QString m_fileName;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // FILEVICE_H
