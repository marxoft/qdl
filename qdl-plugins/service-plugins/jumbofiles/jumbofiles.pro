QT += core network
QT -= gui
TARGET = jumbofiles
TEMPLATE = lib

HEADERS += \
    jumbofiles.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    jumbofiles.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
