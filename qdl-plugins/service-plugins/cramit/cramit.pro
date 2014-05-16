QT += core network
QT -= gui
TARGET = cramit
TEMPLATE = lib

HEADERS += \
    cramit.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    cramit.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
