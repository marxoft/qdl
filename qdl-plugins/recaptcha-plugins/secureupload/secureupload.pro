QT += core network
QT -= gui
TARGET = secureuploadrecaptcha
TEMPLATE = lib

HEADERS += \
    secureupload.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    secureupload.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
