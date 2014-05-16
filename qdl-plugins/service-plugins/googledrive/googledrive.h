#ifndef GOOGLEDRIVE_H
#define GOOGLEDRIVE_H

#include <QObject>
#include <QList>
#include <QUrl>
#include "serviceplugin.h"

class GoogleDrive : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit GoogleDrive(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new GoogleDrive; }
    inline QString iconName() const { return QString("googledrive.jpg"); }
    inline QString serviceName() const { return QString("Google Drive"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private:
    void getDownloadPage(const QUrl &url);
    void getRedirect(const QUrl &url);
    QMap<int, QUrl> getYouTubeVideoUrlMap(QString page);
    QString unescape(const QString &s);

private slots:
    void checkUrlIsValid();
    void checkWebPage();
    void checkDownloadPage();
    void checkRedirect();

signals:
    void currentOperationCancelled();
    
private:
    QList<int> m_formatList;
};

#endif // GOOGLEDRIVE_H
