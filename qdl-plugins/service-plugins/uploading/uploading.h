#ifndef UPLOADING_H
#define UPLOADING_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Uploading : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Uploading(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Uploading; }
    inline QString iconName() const { return QString("uploading.jpg"); }
    inline QString serviceName() const { return QString("Uploading"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

private:
    void getWaitTime();
    void getDownloadPage(const QUrl &url);
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkWaitTime();
    void updateWaitTime();
    void onWaitFinished();
    void getDownloadLink();
    void checkDownloadLink();
    void checkDownloadPage();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_fileId;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // UPLOADING_H
