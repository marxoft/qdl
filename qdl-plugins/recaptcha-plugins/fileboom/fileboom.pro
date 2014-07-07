QT += core network
QT -= gui
TARGET = fileboomrecaptcha
TEMPLATE = lib

HEADERS += \
    fileboom.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    fileboom.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
