QT += core network
QT -= gui
TARGET = datafile
TEMPLATE = lib

HEADERS += \
    datafile.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    datafile.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
