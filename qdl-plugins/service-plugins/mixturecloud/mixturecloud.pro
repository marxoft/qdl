QT += core network
QT -= gui
TARGET = mixturecloud
TEMPLATE = lib

HEADERS += \
    mixturecloud.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    mixturecloud.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
