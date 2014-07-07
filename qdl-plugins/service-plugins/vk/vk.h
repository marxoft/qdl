#ifndef VK_H
#define VK_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class Vk : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Vk(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Vk; }
    inline QString iconName() const { return QString("vk.jpg"); }
    inline QString serviceName() const { return QString("VK"); }
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

#endif // VK_H
