QT += core gui widgets concurrent
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           InputItemDelegate.cpp \
           OutputItemDelegate.cpp \
           Model.cpp

HEADERS += mainwindow.h \
           InputItemDelegate.h \
           OutputItemDelegate.h \
           Model.h

FORMS += mainwindow.ui

INCLUDEPATH += ../core/
DEPENDPATH += . ../core/

LIBS += -L../core/ -L. -lcore

win32 {
    CONFIG(debug, debug|release) {
        LIBS += -L../core/debug/
        PRE_TARGETDEPS += $$OUT_PWD/../core/debug/core.lib
    } else {
        LIBS += -L../core/release/
        PRE_TARGETDEPS += $$OUT_PWD/../core/release/core.lib
    }

    message($$LIBS)
    message($$OUT_PWD)
}


#CONFIG+=debug

TRANSLATIONS += Languages/app_ru.ts

system(lupdate $$_PRO_FILE_)
system(lrelease $$_PRO_FILE_)

RESOURCES += translations.qrc \
             images.qrc

win32:RC_ICONS += Icons/icon.ico
macx:ICON = Icons/icon.icns
