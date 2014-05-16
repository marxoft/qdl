QT += core network
QT -= gui
TARGET = solvemedia
TEMPLATE = lib

HEADERS += \
    solvemedia.h \
    recaptchainterface.h \
    recaptchaplugin.h \
    json.h

SOURCES += \
    solvemedia.cpp \
    json.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
