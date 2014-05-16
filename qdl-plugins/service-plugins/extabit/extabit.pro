QT += core network
QT -= gui
TARGET = extabit
TEMPLATE = lib

HEADERS += \
    extabit.h \
    serviceinterface.h \
    serviceplugin.h \
    json.h

SOURCES += \
    extabit.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
