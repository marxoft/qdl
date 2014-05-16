QT += core network
QT -= gui
TARGET = megashares
TEMPLATE = lib

HEADERS += \
    megashares.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    megashares.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
