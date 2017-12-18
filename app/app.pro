QT += core gui widgets
TEMPLATE = app

SOURCES += main.cpp mainwindow.cpp InputDelegate.cpp

HEADERS += mainwindow.h InputDelegate.h

FORMS += mainwindow.ui

INCLUDEPATH += ../core/
DEPENDPATH += . ../core/

LIBS += -L../core/ -L. -lcore

CONFIG+=debug

TRANSLATIONS += Languages/app_ru.ts

system(lupdate $$_PRO_FILE_)
system(lrelease $$_PRO_FILE_)

RESOURCES += translations.qrc

