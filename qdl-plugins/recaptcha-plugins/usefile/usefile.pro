QT += core network
QT -= gui
TARGET = usefilerecaptcha
TEMPLATE = lib

HEADERS += \
    usefile.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    usefile.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
