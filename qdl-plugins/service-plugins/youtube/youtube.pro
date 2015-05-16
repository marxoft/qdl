QT += core network
QT -= gui
TARGET = youtube
TEMPLATE = lib

HEADERS += \
    youtube.h \
    serviceinterface.h \
    serviceplugin.h

SOURCES += \
    youtube.cpp

icon.files = "$$TARGET".jpg
settings.files = "$$TARGET".xml

unix {
    LIBS += -L/usr/lib -lqyoutube
    CONFIG += link_prl
    PKGCONFIG += libqyoutube
    
    icon.path = /opt/qdl/icons
    settings.path = /opt/qdl/settings
    target.path = /opt/qdl/service_plugins
    INSTALLS += target icon settings
}
