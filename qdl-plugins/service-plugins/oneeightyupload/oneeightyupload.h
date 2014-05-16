#ifndef ONEEIGHTYUPLOAD_H
#define ONEEIGHTYUPLOAD_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class OneEightyUpload : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit OneEightyUpload(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new OneEightyUpload; }
    inline QString iconName() const { return QString("oneeightyupload.jpg"); }
    inline QString serviceName() const { return QString("180Upload"); }
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

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void getDownloadLink();
    void checkDownloadLink();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_fileId;
    QString m_rand;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // ONEEIGHTYUPLOAD_H
