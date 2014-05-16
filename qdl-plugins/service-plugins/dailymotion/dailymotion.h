#ifndef DAILYMOTION_H
#define DAILYMOTION_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include "serviceplugin.h"

class Dailymotion : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit Dailymotion(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new Dailymotion; }
    inline QString iconName() const { return QString("dailymotion.jpg"); }
    inline QString serviceName() const { return QString("Dailymotion"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &webUrl);
    void getDownloadRequest(const QUrl &webUrl);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    inline QString errorString() const { return m_errorString; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private:
    void checkPlaylistVideoUrls(const QUrl &url);
    void getVideoUrl(const QUrl &url);
    inline void setErrorString(const QString &errorString) { m_errorString = errorString; }

private slots:
    void checkUrlIsValid();
    void parseVideoPage();
    void checkVideoUrl();

signals:
    void currentOperationCancelled();

private:
    QStringList m_formatList;
    QString m_errorString;
};

#endif // DAILYMOTION_H
