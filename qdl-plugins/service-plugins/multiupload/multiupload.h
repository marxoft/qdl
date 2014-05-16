#ifndef MULTIUPLOAD_H
#define MULTIUPLOAD_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class Multiupload : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Multiupload(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Multiupload; }
    inline QString iconName() const { return QString("multiupload.jpg"); }
    inline QString serviceName() const { return QString("Multiupload"); }
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
    void checkWebPage();

signals:
    void currentOperationCancelled();
};

#endif // MULTIUPLOAD_H
