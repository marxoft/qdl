#ifndef SERVICEINTERFACE_H
#define SERVICEINTERFACE_H

#include <QtPlugin>

class ServicePlugin;

class ServiceInterface
{

public:
    virtual ~ServiceInterface() {}
    virtual ServicePlugin* getServicePlugin() = 0;
    virtual ServicePlugin* createServicePlugin() = 0;
};

Q_DECLARE_INTERFACE(ServiceInterface, "com.marxoft.QDL.ServiceInterface/1.0")

#endif // SERVICEINTERFACE_H
