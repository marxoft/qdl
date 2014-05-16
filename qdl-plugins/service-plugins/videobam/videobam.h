#ifndef VIDEOBAM_H
#define VIDEOBAM_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class VideoBam : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit VideoBam(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new VideoBam; }
    inline QString iconName() const { return QString("videobam.jpg"); }
    inline QString serviceName() const { return QString("VideoBam"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    bool cancelCurrentOperation();

private:
    void getDownloadLink(const QUrl &url);
    void startWait(int msecs);

private slots:
    void checkUrlIsValid();
    void onWebPageLoaded();
    void checkDownloadLink();
    void updateWaitTime();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QTimer *m_waitTimer;
    int m_waitTime;
};

#endif // VIDEOBAM_H
