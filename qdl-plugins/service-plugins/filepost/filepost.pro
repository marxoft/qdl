QT += core network
QT -= gui
TARGET = filepost
TEMPLATE = lib

HEADERS += \
    filepost.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    filepost.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
