#ifndef GOFOURUP_H
#define GOFOURUP_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class GoFourUp : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit GoFourUp(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new GoFourUp; }
    inline QString iconName() const { return QString("gofourup.jpg"); }
    inline QString serviceName() const { return QString("Go4Up"); }
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

signals:
    void currentOperationCancelled();

private:
    static QHash<QString, QString> filehosts;
    
    QString m_filehost;
};

#endif // GOFOURUP_H
