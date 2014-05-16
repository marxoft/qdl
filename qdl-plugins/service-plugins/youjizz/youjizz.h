#ifndef YOUJIZZ_H
#define YOUJIZZ_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class YouJizz : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit YouJizz(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new YouJizz; }
    inline QString iconName() const { return QString("youjizz.jpg"); }
    inline QString serviceName() const { return QString("YouJizz"); }
    virtual QRegExp urlPattern() const;
    virtual bool urlSupported(const QUrl &url) const;
    virtual void checkUrl(const QUrl &url);
    virtual void getDownloadRequest(const QUrl &url);
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

#endif // YOUJIZZ_H
