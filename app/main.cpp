#include "mainwindow.h"
#include <QApplication>
#include <QtCore>
#include <QtGlobal>
#include <QtDebug>

int main(int argc, char *argv[]) {
  
  QApplication application(argc, argv);
  
  QTranslator translator;
  
    if (translator.load(QLocale(QLocale::Russian),
                        QLatin1String("app"),
                        QLatin1String("_"),
                        QLatin1String(":/Languages"))) {
    application.installTranslator(&translator);
    }
  
  QCoreApplication
    ::setApplicationName("Transportation Problem");
  
  QCoreApplication
    ::setApplicationVersion("1.0.0");
  
  MainWindow w;
  w.show();
  
  return application.exec();
}
