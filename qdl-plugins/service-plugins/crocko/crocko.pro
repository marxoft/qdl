QT += core network
QT -= gui
TARGET = crocko
TEMPLATE = lib

HEADERS += \
    crocko.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    crocko.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
