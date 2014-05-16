QT += core network
QT -= gui
TARGET = uploaded
TEMPLATE = lib

HEADERS += \
    uploaded.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    uploaded.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
