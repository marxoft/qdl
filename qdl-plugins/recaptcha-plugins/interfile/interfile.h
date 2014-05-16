#ifndef INTERFILE_H
#define INTERFILE_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class Interfile : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit Interfile(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new Interfile; }
    inline QString serviceName() const { return QString("Interfile"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // INTERFILE_H
