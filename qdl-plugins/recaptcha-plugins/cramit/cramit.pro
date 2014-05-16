QT += core network
QT -= gui
TARGET = cramitrecaptcha
TEMPLATE = lib

HEADERS += \
    cramit.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    cramit.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
