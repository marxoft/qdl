#ifndef RECAPTCHAINTERFACE_H
#define RECAPTCHAINTERFACE_H

#include <QtPlugin>

class RecaptchaPlugin;

class RecaptchaInterface
{

public:
    virtual ~RecaptchaInterface() {}
    virtual RecaptchaPlugin* getRecaptchaPlugin() = 0;
    virtual RecaptchaPlugin* createRecaptchaPlugin() = 0;
};

Q_DECLARE_INTERFACE(RecaptchaInterface, "com.marxian.QDL.RecaptchaInterface/1.0")

#endif // RECAPTCHAINTERFACE_H
