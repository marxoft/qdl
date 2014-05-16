QT += core network xml
QT -= gui
TARGET = drtuber
TEMPLATE = lib

HEADERS += \
    drtuber.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    drtuber.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
