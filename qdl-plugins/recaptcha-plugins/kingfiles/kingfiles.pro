QT += core network
QT -= gui
TARGET = kingfilesrecaptcha
TEMPLATE = lib

HEADERS += \
    kingfiles.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    kingfiles.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
