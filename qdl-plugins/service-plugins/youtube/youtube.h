#ifndef YOUTUBE_H
#define YOUTUBE_H

#include <QObject>
#include <QList>
#include <QUrl>
#include <QScriptValue>
#include "serviceplugin.h"

class YouTube : public ServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(ServiceInterface)

public:
    explicit YouTube(QObject *parent = 0);
    ServicePlugin* createServicePlugin() { return new YouTube; }
    inline QString iconName() const { return QString("youtube.jpg"); }
    inline QString serviceName() const { return QString("YouTube"); }
    QRegExp urlPattern() const;
    bool urlSupported(const QUrl &url) const;
    void checkUrl(const QUrl &url);
    void getDownloadRequest(const QUrl &url);
    inline bool loginSupported() const { return false; }
    inline bool recaptchaRequired() const { return false; }
    inline int maximumConnections() const { return 0; }
    bool cancelCurrentOperation();

private:
    void checkPlaylistVideoUrls(const QUrl &url);
    void getYouTubeVideoInfoPage(const QString &id);
    void getYouTubeVideoWebPage(const QString &id);
    QMap<int, QUrl> getYouTubeVideoUrlMap(QString page, QScriptValue decryptionFunction = QScriptValue());
    QScriptValue getYouTubeDecryptionFunction(const QUrl &playerUrl);
    QString unescape(const QString &s);

private slots:
    void checkUrlIsValid();
    void checkYouTubeVideoInfoPage();
    void checkYouTubeWebPage();
    void parseYouTubeVideoPage(QScriptValue decryptionFunction = QScriptValue(), QString page = QString());
    void addYouTubeDecryptionFunctionToCache();

signals:
    void youtubeDecryptionFunctionReady(QScriptValue decryptionFunction);
    void currentOperationCancelled();
    
private:
    QString m_youtubePage;
    QList<int> m_formatList;
};

#endif // YOUTUBE_H
