QT += core network
QT -= gui
TARGET = terafile
TEMPLATE = lib

HEADERS += \
    terafile.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    terafile.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
