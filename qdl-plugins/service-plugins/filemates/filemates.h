#ifndef FILEMATES_H
#define FILEMATES_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class FileMates : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit FileMates(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new FileMates; }
    inline QString iconName() const { return QString("filemates.jpg"); }
    inline QString serviceName() const { return QString("FileMates"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

private:
    void startWait(int msecs);
    void getCaptcha();

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkCaptcha();
    void submitCaptcha();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_fileId;
    QString m_fileName;
    QString m_code;
    QString m_rand;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // FILEMATES_H
