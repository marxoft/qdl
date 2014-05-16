QT += core network
QT -= gui
TARGET = youjizz
TEMPLATE = lib

HEADERS += \
    youjizz.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    youjizz.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
