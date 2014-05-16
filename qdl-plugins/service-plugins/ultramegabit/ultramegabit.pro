QT += core network
QT -= gui
TARGET = ultramegabit
TEMPLATE = lib

HEADERS += \
    ultramegabit.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    ultramegabit.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
