QT += core network
QT -= gui
TARGET = upstore
TEMPLATE = lib

HEADERS += \
    upstore.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    upstore.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
