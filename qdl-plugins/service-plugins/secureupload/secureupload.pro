QT += core network
QT -= gui
TARGET = secureupload
TEMPLATE = lib

HEADERS += \
    secureupload.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    secureupload.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
