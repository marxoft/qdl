QT += core network
QT -= gui
TARGET = dailymotion
TEMPLATE = lib

HEADERS += \
    dailymotion.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    dailymotion.cpp \
    json.cpp

icon.files = "$$TARGET".jpg
settings.files = "$$TARGET".xml

unix {
    icon.path = /opt/qdl/icons
    settings.path = /opt/qdl/settings
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon settings
}
