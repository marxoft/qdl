QT += core network
QT -= gui
TARGET = ulozrecaptcha
TEMPLATE = lib

HEADERS += \
    uloz.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    uloz.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
