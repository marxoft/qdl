QT += core network
QT -= gui
TARGET = babevideo
TEMPLATE = lib

HEADERS += \
    babevideo.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    babevideo.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
