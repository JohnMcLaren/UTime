#-------------------------------------------------
#
# Project created by QtCreator 2017-11-06T08:13:28
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UTime
TEMPLATE = app


SOURCES += main.cpp\
    cthread.cpp \
        mainwindow.cpp \
    qntp/NtpReply.cpp \
    qntp/NtpClient.cpp

HEADERS  += mainwindow.h \
    cthread.h \
    qntp/QNtp.h \
    qntp/config.h \
    qntp/NtpClient.h \
    qntp/NtpPacket.h \
    qntp/NtpReply.h \
    qntp/NtpReply_p.h \
    qntp/NtpTimestamp.h

FORMS    += mainwindow.ui

DISTFILES += \
    License.txt
