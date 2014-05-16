QT += core network xml script
QT -= gui
TARGET = youtube
TEMPLATE = lib

HEADERS += \
    youtube.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    youtube.cpp \
    json.cpp

icon.files = "$$TARGET".jpg
settings.files = "$$TARGET".xml

unix {
    icon.path = /opt/qdl/icons
    settings.path = /opt/qdl/settings
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon settings
}
