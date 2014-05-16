#ifndef XHAMSTER_H
#define XHAMSTER_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class XHamster : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit XHamster(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new XHamster; }
    inline QString iconName() const { return QString("xhamster.jpg"); }
    inline QString serviceName() const { return QString("XHamster"); }
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

#endif // XHAMSTER_H
