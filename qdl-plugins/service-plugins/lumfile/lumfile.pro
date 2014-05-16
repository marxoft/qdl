QT += core network
QT -= gui
TARGET = lumfile
TEMPLATE = lib

HEADERS += \
    lumfile.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    lumfile.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
