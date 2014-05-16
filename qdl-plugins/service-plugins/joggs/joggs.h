#ifndef JOGGS_H
#define JOGGS_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class Joggs : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Joggs(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Joggs; }
    inline QString iconName() const { return QString("joggs.jpg"); }
    inline QString serviceName() const { return QString("Joggs"); }
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

#endif // JOGGS_H
