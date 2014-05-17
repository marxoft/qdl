QT += core network
QT -= gui
TARGET = uploadable
TEMPLATE = lib

HEADERS += \
    uploadable.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    uploadable.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
