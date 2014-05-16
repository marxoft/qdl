#ifndef CLOUDSTORES_H
#define CLOUDSTORES_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class CloudStores : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit CloudStores(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new CloudStores; }
    inline QString iconName() const { return QString("cloudstores.jpg"); }
    inline QString serviceName() const { return QString("CloudStor.es"); }
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
    void checkDownloadPage();

signals:
    void currentOperationCancelled();
};

#endif // CLOUDSTORES_H
