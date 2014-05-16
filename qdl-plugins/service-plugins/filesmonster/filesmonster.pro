QT += core network
QT -= gui
TARGET = filesmonster
TEMPLATE = lib

HEADERS += \
    filesmonster.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    filesmonster.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
