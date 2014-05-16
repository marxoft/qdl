#ifndef RAMPANTTV_H
#define RAMPANTTV_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class RampantTV : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit RampantTV(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new RampantTV; }
    inline QString iconName() const { return QString("rampanttv.jpg"); }
    inline QString serviceName() const { return QString("RampantTV"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private:
    void getVideoParams(const QUrl &url);

private slots:
    void checkUrlIsValid();
    void parseVideoPage();
    void parseVideoParams();

signals:
    void currentOperationCancelled();
};

#endif // RAMPANTTV_H
