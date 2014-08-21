QT += core network
QT -= gui
TARGET = videograbber
TEMPLATE = lib

HEADERS += \
    videograbber.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    videograbber.cpp

settings.files = "$$TARGET".xml
icon.files = "$$TARGET".jpg

unix {
    settings.path = /opt/qdl/settings
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target settings icon
}
