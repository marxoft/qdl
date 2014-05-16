#ifndef ASFILE_H
#define ASFILE_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class ASfile : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit ASfile(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new ASfile; }
    inline QString iconName() const { return QString("asfile.jpg"); }
    inline QString serviceName() const { return QString("ASfile"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return true; }
    void login(const QString &username, const QString &password);
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline int maximumConnections() const { return m_connections; }
    inline bool downloadsAreResumable() const { return m_connections != 1; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void onWebPageDownloaded();
    void downloadCaptcha();
    void getDownloadPage();
    void checkDownloadPage();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void convertHashToLink();
    void checkDownloadLink();
    void onWaitFinished();

signals:
    void currentOperationCancelled();

private:
    QString m_fileId;
    QString m_fileName;
    QString m_hash;
    QString m_storage;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // RAPIDGATOR_H
