#ifndef BLIPTV_H
#define BLIPTV_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class Bliptv : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Bliptv(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Bliptv; }
    inline QString iconName() const { return QString("bliptv.jpg"); }
    inline QString serviceName() const { return QString("Blip.tv"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    bool cancelCurrentOperation();
    
private:
    void getRedirect(const QUrl &url);

private slots:
    void checkUrlIsValid();
    void parseVideoPage();
    void checkRedirect();

signals:
    void currentOperationCancelled();
};

#endif // BLIPTV_H
