QT += core network
QT -= gui
TARGET = netloadrecaptcha
TEMPLATE = lib

HEADERS += \
    netload.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    netload.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
