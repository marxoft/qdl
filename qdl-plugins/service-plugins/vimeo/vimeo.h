#ifndef VIMEO_H
#define VIMEO_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include "serviceplugin.h"

class Vimeo : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Vimeo(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Vimeo; }
    inline QString iconName() const { return QString("vimeo.jpg"); }
    inline QString serviceName() const { return QString("Vimeo"); }
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

private:
    QStringList m_formatList;
};

#endif // VIMEO_H
