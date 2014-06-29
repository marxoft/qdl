#ifndef PORNTUBE_H
#define PORNTUBE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class PornTube : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit PornTube(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new PornTube; }
    inline QString iconName() const { return QString("porntube.jpg"); }
    inline QString serviceName() const { return QString("PornTube"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private:
    void getVideoParams(const QString &id);

private slots:
    void checkUrlIsValid();
    void parseVideoPage();
    void parseVideoParams();

signals:
    void currentOperationCancelled();
};

#endif // PORNTUBE_H
