QT += core network
QT -= gui
TARGET = rampanttv
TEMPLATE = lib

HEADERS += \
    rampanttv.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    rampanttv.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
