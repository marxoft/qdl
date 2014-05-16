#ifndef FILERIO_H
#define FILERIO_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class FileRio : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit FileRio(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new FileRio; }
    inline QString iconName() const { return QString("filerio.jpg"); }
    inline QString serviceName() const { return QString("FileRio"); }
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
    void getPageTwo();
    void getPageThree();
    QString unescape(const QString &s);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkPageTwo();
    void checkPageThree();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_fileId;
    QString m_fileName;
    QString m_rand;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // FILERIO_H
