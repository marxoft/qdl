QT += core network
QT -= gui
TARGET = kingfiles
TEMPLATE = lib

HEADERS += \
    kingfiles.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    kingfiles.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
