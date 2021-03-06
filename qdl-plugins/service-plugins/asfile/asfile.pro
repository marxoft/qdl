QT += core network
QT -= gui
TARGET = asfile
TEMPLATE = lib

HEADERS += \
    asfile.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    asfile.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
