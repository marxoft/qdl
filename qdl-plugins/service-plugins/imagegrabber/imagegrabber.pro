QT += core network
QT -= gui
TARGET = imagegrabber
TEMPLATE = lib

HEADERS += \
    imagegrabber.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    imagegrabber.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
