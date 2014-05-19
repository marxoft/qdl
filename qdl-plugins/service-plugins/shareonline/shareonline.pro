QT += core network
QT -= gui
TARGET = shareonline
TEMPLATE = lib

HEADERS += \
    shareonline.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    shareonline.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
