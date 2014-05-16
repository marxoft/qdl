QT += core network
QT -= gui
TARGET = rapidgator
TEMPLATE = lib

HEADERS += \
    rapidgator.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    rapidgator.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
