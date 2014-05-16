#ifndef DECAPTCHAINTERFACE_H
#define DECAPTCHAINTERFACE_H

#include <QtPlugin>

class DecaptchaPlugin;

class DecaptchaInterface
{

public:
    virtual ~DecaptchaInterface() {}
    virtual DecaptchaPlugin* getDecaptchaPlugin() = 0;
    virtual DecaptchaPlugin* createDecaptchaPlugin() = 0;
};

Q_DECLARE_INTERFACE(DecaptchaInterface, "com.marxoft.QDL.DecaptchaInterface/1.0")

#endif // SERVICEINTERFACE_H
