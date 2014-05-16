#ifndef BABEVIDEO_H
#define BABEVIDEO_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class BabeVideo : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit BabeVideo(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new BabeVideo; }
    inline QString iconName() const { return QString("babevideo.jpg"); }
    inline QString serviceName() const { return QString("Babe Video"); }
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

#endif // BABEVIDEO_H
