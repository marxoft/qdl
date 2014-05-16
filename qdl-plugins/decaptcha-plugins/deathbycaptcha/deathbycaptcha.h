#ifndef DEATHBYCAPTCHA_H
#define DEATHBYCAPTCHA_H

#include "decaptchaplugin.h"
#include "formpost.h"
#include <QObject>
#include <QUrl>

class DeathByCaptcha : public DecaptchaPlugin
{
    Q_OBJECT
    Q_INTERFACES(DecaptchaInterface)

public:
    explicit DeathByCaptcha(QObject *parent = 0);
    DecaptchaPlugin* createDecaptchaPlugin() { return new DeathByCaptcha; }
    inline QString iconName() const { return QString("deathbycaptcha.jpg"); }
    inline QString serviceName() const { return QString("DeathByCaptcha"); }
    inline QString captchaId() const { return m_captchaId; }
    void getCaptchaResponse(const QByteArray &data);
    void reportIncorrectCaptchaResponse(const QString &id);
    bool cancelCurrentOperation();

private:
    void reportError(int errorCode);

private slots:
    void onCaptchaSubmitted();
    void checkCaptchaStatus();
    void checkCaptchaStatusResponse();
    void onCaptchaReported();

signals:
    void currentOperationCancelled();

private:
    QUrl m_statusUrl;
    QString m_captchaId;
    FormPostPlugin *m_formPost;
};

#endif // DEATHBYCAPTCHA_H
