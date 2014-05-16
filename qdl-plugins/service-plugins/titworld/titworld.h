#ifndef TITWORLD_H
#define TITWORLD_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class TitWorld : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit TitWorld(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new TitWorld; }
    inline QString iconName() const { return QString("titworld.jpg"); }
    inline QString serviceName() const { return QString("TitWorld"); }
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

#endif // TITWORLD_H
