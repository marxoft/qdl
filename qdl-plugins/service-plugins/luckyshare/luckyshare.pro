QT += core network
QT -= gui
TARGET = luckyshare
TEMPLATE = lib

HEADERS += \
    luckyshare.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    luckyshare.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
