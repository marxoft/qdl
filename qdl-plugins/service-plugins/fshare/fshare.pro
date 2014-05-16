QT += core network
QT -= gui
TARGET = fshare
TEMPLATE = lib

HEADERS += \
    fshare.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    fshare.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
