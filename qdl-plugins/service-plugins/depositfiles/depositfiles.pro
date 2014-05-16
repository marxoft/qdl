QT += core network script
QT -= gui
TARGET = depositfiles
TEMPLATE = lib

HEADERS += \
    depositfiles.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    depositfiles.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
