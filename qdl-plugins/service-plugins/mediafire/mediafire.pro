QT += core network
QT -= gui
TARGET = mediafire
TEMPLATE = lib

HEADERS += \
    mediafire.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    mediafire.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
