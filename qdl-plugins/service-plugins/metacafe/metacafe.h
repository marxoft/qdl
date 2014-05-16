#ifndef METACAFE_H
#define METACAFE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class Metacafe : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Metacafe(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Metacafe; }
    inline QString iconName() const { return QString("metacafe.jpg"); }
    inline QString serviceName() const { return QString("Metacafe"); }
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

#endif // METACAFE_H
