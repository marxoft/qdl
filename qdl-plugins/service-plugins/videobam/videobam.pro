QT += core gui network
TARGET = videobam
TEMPLATE = lib

HEADERS += \
    videobam.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    videobam.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
