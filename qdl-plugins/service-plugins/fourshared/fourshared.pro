QT += core network
QT -= gui
TARGET = fourshared
TEMPLATE = lib

HEADERS += \
    fourshared.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    fourshared.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
