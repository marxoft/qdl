QT += core network
QT -= gui
TARGET = interfilerecaptcha
TEMPLATE = lib

HEADERS += \
    interfile.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    interfile.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
