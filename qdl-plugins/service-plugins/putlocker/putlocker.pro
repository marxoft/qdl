QT += core network
QT -= gui
TARGET = putlocker
TEMPLATE = lib

HEADERS += \
    putlocker.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    putlocker.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
