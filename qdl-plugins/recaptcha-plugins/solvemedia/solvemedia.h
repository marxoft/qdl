#ifndef SOLVEMEDIA_H
#define SOLVEMEDIA_H

#include "recaptchaplugin.h"
#include <QObject>

class QNetworkAccessManager;
class QUrl;

class SolveMedia : public RecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(RecaptchaInterface)

public:
    explicit SolveMedia(QObject *parent = 0);
    inline SolveMedia* createRecaptchaPlugin() { return new SolveMedia; }
    inline QString serviceName() const { return QString("SolveMedia"); }
    void getCaptcha(const QString &key);
    bool cancelCurrentOperation();

private:
    void downloadCaptchaImage(const QString &challenge);
    void reportError(int errorCode);

private slots:
    void onCaptchaDownloaded();
    void onCaptchaImageDownloaded();

signals:
    void currentOperationCancelled();
};

#endif // SOLVEMEDIA_H
