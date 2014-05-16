QT += core network
QT -= gui
TARGET = google
TEMPLATE = lib

HEADERS += \
    google.h \
    recaptchainterface.h \
    recaptchaplugin.h

SOURCES += \
    google.cpp

unix {
    target.path = /opt/qdl/recaptcha_plugins
    INSTALLS += target
}
