#ifndef DRTUBER_H
#define DRTUBER_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class DrTuber : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit DrTuber(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new DrTuber; }
    inline QString iconName() const { return QString("drtuber.jpg"); }
    inline QString serviceName() const { return QString("DrTuber"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();
    
private:
    void getVideoUrl(const QUrl &url);

private slots:
    void checkUrlIsValid();
    void parseVideoPage();
    void checkVideoUrl();

signals:
    void currentOperationCancelled();
};

#endif // DRTUBER_H
