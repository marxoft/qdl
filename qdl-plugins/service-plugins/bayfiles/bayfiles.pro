QT += core network
QT -= gui
TARGET = bayfiles
TEMPLATE = lib

HEADERS += \
    bayfiles.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    bayfiles.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
