#ifndef FILEBOOM_H
#define FILEBOOM_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class FileBoom : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit FileBoom(QObject *parent = 0);
    inline RecaptchaPlugin* createRecaptchaPlugin() { return new FileBoom; }
    inline QString serviceName() const { return QString("FileBoom"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // FILEBOOM_H
