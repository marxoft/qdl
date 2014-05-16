#ifndef TVGIRLSPLAZA_H
#define TVGIRLSPLAZA_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class TVGirlsPlaza : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit TVGirlsPlaza(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new TVGirlsPlaza; }
    inline QString iconName() const { return QString("tvgirlsplaza.jpg"); }
    inline QString serviceName() const { return QString("TVGirlsPlaza"); }
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

#endif // TVGIRLSPLAZA_H
