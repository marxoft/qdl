#ifndef SOCKSHARE_H
#define SOCKSHARE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class SockShare : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit SockShare(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new SockShare; }
    inline QString iconName() const { return QString("sockshare.jpg"); }
    inline QString serviceName() const { return QString("SockShare"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private:
    void getDownloadLink(const QUrl &url, const QByteArray &hash);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void checkDownloadLink();

signals:
    void currentOperationCancelled();
};

#endif // SOCKSHARE_H
