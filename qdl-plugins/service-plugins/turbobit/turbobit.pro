QT += core network
QT -= gui
TARGET = turbobit
TEMPLATE = lib

INCLUDEPATH += \
    ../interfaces

HEADERS += \
    turbobit.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    turbobit.cpp

maemo5 {
    target.path = /opt/qdl/bin/service_plugins
    INSTALLS += target
}

contains(MEEGO_EDITION,harmattan) {
    target.path = /opt/qdl/bin/service_plugins
    INSTALLS += target
}
