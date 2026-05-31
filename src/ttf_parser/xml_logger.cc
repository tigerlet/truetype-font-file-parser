#include "stdafx.h"
#include "xml_logger.h"
#include <cstdarg>

namespace ttf_dll {

XmlLogger::XmlLogger(const QString &out_path)
  : indent_(0), error_(false) {
  file = new QFile(out_path);
  if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
    error_ = true;
    return;
  }
  stream = new QTextStream(file);
  stream->setEncoding(QStringConverter::Utf8);
}

XmlLogger::~XmlLogger() {
  if (!error_) {
    stream->flush();
    file->close();
  }
  delete stream;
  delete file;
}

void XmlLogger::PrintIndent() const {
  (*stream) << QString().fill(' ', indent_);
}

void XmlLogger::Print(const char *format, ...) const {
  PrintIndent();
  va_list args;
  va_start(args, format);
  (*stream) << QString().vasprintf(format, args);
  va_end(args);
}

void XmlLogger::Println(const char *format, ...) const {
  PrintIndent();
  va_list args;
  va_start(args, format);
  (*stream) << QString().vasprintf(format, args) << "\n";
  va_end(args);
}

void XmlLogger::WPrint(const wchar_t *format, ...) const {
  PrintIndent();
  QString str = QString::fromWCharArray(format);
  (*stream) << str;
}

void XmlLogger::WPrintln(const wchar_t *format, ...) const {
  PrintIndent();
  QString str = QString::fromWCharArray(format);
  (*stream) << str << "\n";
}

}
