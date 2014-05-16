#ifndef JUMBOFILES_H
#define JUMBOFILES_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class JumboFiles : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit JumboFiles(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new JumboFiles; }
    inline QString iconName() const { return QString("jumbofiles.jpg"); }
    inline QString serviceName() const { return QString("JumboFiles"); }
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
    void getSecondPage();
    void getDownloadPage();
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void updateWaitTime();
    void onWaitFinished();
    void checkSecondPage();
    void checkDownloadPage();

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

#endif // JUMBOFILES_H
