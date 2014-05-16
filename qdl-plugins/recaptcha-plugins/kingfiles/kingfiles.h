#ifndef KINGFILES_H
#define KINGFILES_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class Kingfiles : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit Kingfiles(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new Kingfiles; }
    inline QString serviceName() const { return QString("Kingfiles"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // KINGFILES_H
