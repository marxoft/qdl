QT += core network
QT -= gui
TARGET = turbobitrecaptcha
TEMPLATE = lib

HEADERS += \
    turbobit.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    turbobit.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
