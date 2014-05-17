QT += core network
QT -= gui
TARGET = ulozrecaptcha
TEMPLATE = lib

HEADERS += \
    uloz.h \
    recaptchainterface.h \
    recaptchaplugin.h \
    json.h

SOURCES += \
    uloz.cpp \
    json.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
