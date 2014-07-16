#ifndef GIGAPETA_H
#define GIGAPETA_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class GigaPeta : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit GigaPeta(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new GigaPeta; }
    inline QString serviceName() const { return QString("GigaPeta"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // GIGAPETA_H
