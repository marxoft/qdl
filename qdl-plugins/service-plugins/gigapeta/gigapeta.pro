QT += core network
QT -= gui
TARGET = gigapeta
TEMPLATE = lib

HEADERS += \
    gigapeta.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    gigapeta.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
