#ifndef ULOZ_H
#define ULOZ_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class Uloz : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit Uloz(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new Uloz; }
    inline QString serviceName() const { return QString("Uloz"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // ULOZ_H
