QT += core network
QT -= gui
TARGET = sharevnn
TEMPLATE = lib

HEADERS += \
    sharevnn.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    sharevnn.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
