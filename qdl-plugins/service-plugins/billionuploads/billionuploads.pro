QT += core network
QT -= gui
TARGET = billionuploads
TEMPLATE = lib

HEADERS += \
    billionuploads.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    billionuploads.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
