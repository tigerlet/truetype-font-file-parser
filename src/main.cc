#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include "ui/main_window.h"

void debugLog(const QString &message) {
  qDebug() << message;
  QFile logFile("c:/Users/xqxym/Desktop/ttf_parser-master/qt_ttf_parser/build_clean/app_debug.log");
  if (logFile.open(QIODevice::Append | QIODevice::Text)) {
    QTextStream out(&logFile);
    out << message << Qt::endl;
  }
}

int main(int argc, char *argv[]) {
  QFile logFile("c:/Users/xqxym/Desktop/ttf_parser-master/qt_ttf_parser/build_clean/app_debug.log");
  logFile.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&logFile);
  out << "[DEBUG] main: Starting TTF Parser application" << Qt::endl;
  
  debugLog("[DEBUG] main: Starting TTF Parser application");
  
  QApplication app(argc, argv);
  debugLog("[DEBUG] main: QApplication created");
  
  app.setApplicationName("TTF Parser - Qt");
  debugLog("[DEBUG] main: Application name set");
  
  debugLog("[DEBUG] main: Creating MainWindow");
  MainWindow window;
  debugLog("[DEBUG] main: MainWindow created");
  
  debugLog("[DEBUG] main: Calling window.show()");
  window.show();
  debugLog("[DEBUG] main: MainWindow shown");
  
  debugLog("[DEBUG] main: Before app.exec()");
  int result = app.exec();
  debugLog("[DEBUG] main: Application exited with code: " + QString::number(result));
  
  logFile.close();
  return result;
}