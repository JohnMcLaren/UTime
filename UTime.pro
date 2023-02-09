#-------------------------------------------------
#
# Project created by QtCreator 2017-11-06T08:13:28
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UTime
VERSION = 1.3
TEMPLATE = app
DEFINES += APP_NAME=\\\"$$TARGET\\\"
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += main.cpp\
    beep.cpp \
        mainwindow.cpp \
    qntp/NtpReply.cpp \
    qntp/NtpClient.cpp

HEADERS  += mainwindow.h \
    beep.h \
    qntp/QNtp.h \
    qntp/config.h \
    qntp/NtpClient.h \
    qntp/NtpPacket.h \
    qntp/NtpReply.h \
    qntp/NtpReply_p.h \
    qntp/NtpTimestamp.h

FORMS    += mainwindow.ui

# Icon made by Freepik from www.flaticon.com
RC_ICONS ="img/icon.ico"

RESOURCES += \
    resources.qrc

DISTFILES += \
    License.txt

# Set build out directory
CONFIG(debug, debug|release) {
    DESTDIR = debug
} else {
    DESTDIR = release
}
# For objects
OBJECTS_DIR = $$DESTDIR/.obj
# For MOC
MOC_DIR = $$DESTDIR/.moc/



