#ifndef BESTREAMS_H
#define BESTREAMS_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class Bestreams : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Bestreams(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Bestreams; }
    inline QString iconName() const { return QString("bestreams.jpg"); }
    inline QString serviceName() const { return QString("BESTREAMS"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private:
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void getPageTwo();
    void checkPageTwo();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QString m_id;
    QString m_fname;
    QString m_hash;
    QString m_human;
    QTimer *m_waitTimer;
    int m_waitTime;
};

#endif // BESTREAMS_H
