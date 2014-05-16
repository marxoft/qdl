#ifndef FILEVICE_H
#define FILEVICE_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class FileVice : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit FileVice(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new FileVice; }
    inline QString serviceName() const { return QString("FileVice"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // FILEVICE_H
