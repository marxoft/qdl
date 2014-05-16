QT += core network gui
TARGET = deathbycaptcha
TEMPLATE = lib

HEADERS += \
    deathbycaptcha.h \
    decaptchainterface.h \
    decaptchaplugin.h \
    json.h \
    formpost.h

SOURCES += \
    deathbycaptcha.cpp \
    formpost.cpp \
    json.cpp

icon.files = "$$TARGET".jpg

unix {
    icon.path = /opt/qdl/icons
    target.path = /opt/qdl/decaptcha_plugins
    INSTALLS += target icon
}
