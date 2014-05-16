QT += core network
QT -= gui
TARGET = filevicerecaptcha
TEMPLATE = lib

HEADERS += \
    filevice.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    filevice.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
