#ifndef NETLOAD_H
#define NETLOAD_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class Netload : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit Netload(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new Netload; }
    inline QString serviceName() const { return QString("Netload"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // NETLOAD_H
