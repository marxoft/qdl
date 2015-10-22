QT += core network
QT -= gui
TARGET = googledrive
TEMPLATE = lib

HEADERS += \
    googledrive.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    googledrive.cpp

icon.files = "$$TARGET".jpg
settings.files = "$$TARGET".xml

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    settings.path = /opt/qdl/settings
    INSTALLS += target icon settings
}
