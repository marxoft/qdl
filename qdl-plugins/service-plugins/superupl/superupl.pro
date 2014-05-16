QT += core network
QT -= gui
TARGET = superupl
TEMPLATE = lib

HEADERS += \
    superupl.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    superupl.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
