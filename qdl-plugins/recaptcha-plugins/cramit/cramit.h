#ifndef CRAMIT_H
#define CRAMIT_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class Cramit : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit Cramit(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new Cramit; }
    inline QString serviceName() const { return QString("Cramit"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // CRAMIT_H
