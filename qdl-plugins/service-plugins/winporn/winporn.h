#ifndef WINPORN_H
#define WINPORN_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class WinPorn : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit WinPorn(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new WinPorn; }
    inline QString iconName() const { return QString("winporn.jpg"); }
    inline QString serviceName() const { return QString("WinPorn"); }
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

#endif // WINPORN_H
