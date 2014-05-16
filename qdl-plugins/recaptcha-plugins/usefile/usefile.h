#ifndef USEFILE_H
#define USEFILE_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class UseFile : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit UseFile(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new UseFile; }
    inline QString serviceName() const { return QString("UseFile"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // USEFILE_H
