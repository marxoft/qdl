#ifndef MEGASHARES_H
#define MEGASHARES_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class MegaShares : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit MegaShares(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new MegaShares; }
    inline QString serviceName() const { return QString("MegaShares"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // MEGASHARES_H
