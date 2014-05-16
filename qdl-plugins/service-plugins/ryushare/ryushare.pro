QT += core network
QT -= gui
TARGET = ryushare
TEMPLATE = lib

HEADERS += \
    ryushare.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    ryushare.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
