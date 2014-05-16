QT += core network
QT -= gui
TARGET = tvgirlsplaza
TEMPLATE = lib

HEADERS += \
    tvgirlsplaza.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    tvgirlsplaza.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
