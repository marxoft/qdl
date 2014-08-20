QT += core network
QT -= gui
TARGET = keeptosharerecaptcha
TEMPLATE = lib

HEADERS += \
    keeptoshare.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    keeptoshare.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
