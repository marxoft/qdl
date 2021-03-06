QT += core network
QT -= gui
TARGET = filefactory
TEMPLATE = lib

HEADERS += \
    filefactory.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    filefactory.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
