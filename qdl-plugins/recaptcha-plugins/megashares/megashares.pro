QT += core network
QT -= gui
TARGET = megasharesrecaptcha
TEMPLATE = lib

HEADERS += \
    megashares.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    megashares.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
