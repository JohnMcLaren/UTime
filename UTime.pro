#-------------------------------------------------
#
# Project created by QtCreator 2017-11-06T08:13:28
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UTime
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qntp/NtpReply.cpp \
    qntp/NtpClient.cpp

HEADERS  += mainwindow.h \
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

