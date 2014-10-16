QT += core network
QT -= gui
TARGET = uploadtous
TEMPLATE = lib

HEADERS += \
    uploadtous.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    uploadtous.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
