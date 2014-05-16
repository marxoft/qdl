#ifndef GOOGLE_H
#define GOOGLE_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class Google : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit Google(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new Google; }
    inline QString serviceName() const { return QString("Google"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void downloadCaptchaImage(const QString &challenge);
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();
    void onCaptchaImageDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // GOOGLE_H
