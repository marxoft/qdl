#ifndef FILESMONSTER_H
#define FILESMONSTER_H

#include <QObject>
#include <QUrl>
#include "serviceplugin.h"

class QTimer;

class FilesMonster : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit FilesMonster(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new FilesMonster; }
    inline QString iconName() const { return QString("filesmonster.jpg"); }
    inline QString serviceName() const { return QString("FilesMonster"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    void login(const QString &username, const QString &password);
    inline bool loginSupported() const { return true; }
    inline bool recaptchaRequired() const { return true; }
    inline QString recaptchaKey() const { return m_captchaKey; }
    inline int maximumConnections() const { return m_connections; }
    bool cancelCurrentOperation();

public slots:
    void submitCaptchaResponse(const QString &challenge, const QString &response);

private:
    void getMultipartPage(const QUrl &url);
    void getMultipartJson(const QUrl &url);
    void startWait(int msecs);

private slots:
    void checkLogin();
    void checkUrlIsValid();
    void checkMultipartPage();
    void checkMultipartJson();
    void onWebPageDownloaded();
    void onCaptchaSubmitted();
    void updateWaitTime();
    void onWaitFinished();
    void getDownloadLink();
    void checkDownloadLink();

signals:
    void currentOperationCancelled();

private:
    QString m_downloadLink;
    QUrl m_url;
    QString m_fileName;
    QString m_captchaKey;
    QTimer *m_waitTimer;
    int m_waitTime;
    int m_connections;
};

#endif // FILESMONSTER_H
