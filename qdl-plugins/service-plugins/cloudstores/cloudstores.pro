QT += core network
QT -= gui
TARGET = cloudstores
TEMPLATE = lib

HEADERS += \
    cloudstores.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    cloudstores.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon
}
