QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    book.cpp \
    filesystem.cpp \
    friend.cpp \
    main.cpp \
    netdisktcpclient.cpp \
    online.cpp \
    operatewidget.cpp \
    privatechatwid.cpp \
    protocol.cpp \
    sharedfilefriendlist.cpp

HEADERS += \
    book.h \
    filesystem.h \
    friend.h \
    netdisktcpclient.h \
    online.h \
    operatewidget.h \
    privatechatwid.h \
    protocol.h \
    sharedfilefriendlist.h

FORMS += \
    netdisktcpclient.ui \
    online.ui \
    privatechatwid.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    config.qrc
