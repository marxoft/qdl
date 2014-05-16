#ifndef PINKTUBE_H
#define PINKTUBE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class PinkTube : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit PinkTube(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new PinkTube; }
    inline QString iconName() const { return QString("pinktube.jpg"); }
    inline QString serviceName() const { return QString("PinkTube"); }
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
    void parseVideoPage();

signals:
    void currentOperationCancelled();
};

#endif // PINKTUBE_H
