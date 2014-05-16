#ifndef SOUNDCLOUD_H
#define SOUNDCLOUD_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class SoundCloud : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit SoundCloud(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new SoundCloud; }
    inline QString iconName() const { return QString("soundcloud.jpg"); }
    inline QString serviceName() const { return QString("SoundCloud"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private slots:
    void checkUrlIsValid();
    void onWebPageDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // SOUNDCLOUD_H
