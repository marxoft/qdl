#ifndef MIXTURECLOUD_H
#define MIXTURECLOUD_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class MixtureCloud : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit MixtureCloud(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new MixtureCloud; }
    inline QString iconName() const { return QString("mixturecloud.jpg"); }
    inline QString serviceName() const { return QString("MixtureCloud"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    bool cancelCurrentOperation();

private slots:
    void checkUrlIsValid();
    void onWebPageDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // MIXTURECLOUD_H
