QT += core network
QT -= gui
TARGET = porntube
TEMPLATE = lib

HEADERS += \
    porntube.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    porntube.cpp \
    json.cpp

icon.files = "$$TARGET".jpg
settings.files = "$$TARGET".xml

unix {
    icon.path = /opt/qdl/icons
    settings.path = /opt/qdl/settings
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon settings
}
