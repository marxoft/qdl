#ifndef TURBOBIT_H
#define TURBOBIT_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class TurboBit : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit TurboBit(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new TurboBit; }
    inline QString serviceName() const { return QString("TurboBit"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // TURBOBIT_H
