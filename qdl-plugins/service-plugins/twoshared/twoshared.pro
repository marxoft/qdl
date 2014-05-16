QT += core network
QT -= gui
TARGET = twoshared
TEMPLATE = lib

HEADERS += \
    twoshared.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    twoshared.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
