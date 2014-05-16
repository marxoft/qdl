#ifndef HUGEFILES_H
#define HUGEFILES_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class HugeFiles : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit HugeFiles(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new HugeFiles; }
    inline QString iconName() const { return QString("hugefiles.jpg"); }
    inline QString serviceName() const { return QString("HugeFiles"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaServiceName() const { return m_captchaService; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline QString errorString() const { return m_errorString; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void getDownloadUrl();
    inline void setErrorString(const QString &errorString) { m_errorString = errorString; }

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();
    void checkDownloadUrl();

signals:
    void currentOperationCancelled();

private:
    QString m_fileId;
    QString m_fileName;
    QString m_rand;
    QString m_captchaType;
    QString m_captchaService;
    QString m_captchaKey;
    int m_connections;
    QString m_errorString;
};

#endif // HUGEFILES_H
