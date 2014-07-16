QT += core network
QT -= gui
TARGET = gigapetarecaptcha
TEMPLATE = lib

HEADERS += \
    gigapeta.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    gigapeta.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
