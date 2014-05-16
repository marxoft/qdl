#ifndef MEDIAFIRE_H
#define MEDIAFIRE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class MediaFire : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit MediaFire(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new MediaFire; }
    inline QString iconName() const { return QString("mediafire.jpg"); }
    inline QString serviceName() const { return QString("MediaFire"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline QString recaptchaServiceName() const { return m_captchaService; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void getFolderFileLinks(const QString &folderId);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void checkFolderFileLinks();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();

signals:
    void currentOperationCancelled();

private:
    QUrl m_url;
    QUrl m_captchaUrl;
    QString m_captchaService;
    QString m_captchaKey;
};

#endif // MEDIAFIRE_H
