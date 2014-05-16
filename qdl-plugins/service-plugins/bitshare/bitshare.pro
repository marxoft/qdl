QT += core network
QT -= gui
TARGET = bitshare
TEMPLATE = lib

HEADERS += \
    bitshare.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    bitshare.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
