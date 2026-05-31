#ifndef XML_LOGGER_H
#define XML_LOGGER_H
#include <QString>
#include <QFile>
#include <QTextStream>

namespace ttf_dll {

class XmlLogger {
public:
  explicit XmlLogger(const QString &out_path);
  ~XmlLogger();
  bool Error() const { return error_; }
  void IncreaseIndent() { indent_ += 2; }
  void DecreaseIndent() { indent_ -= 2; }
  void Print(const char *format, ...) const;
  void Println(const char *format, ...) const;
  void WPrint(const wchar_t *format, ...) const;
  void WPrintln(const wchar_t *format, ...) const;
  template<typename T>
  void PrintArray(const void *array, size_t len, const char *format) const {
    PrintIndent();
    T *ptr = (T*)array;
    for(size_t i = 0; i < len; ++i) {
      (*stream) << QString().asprintf(format, *ptr++);
      if(i == len - 1) {
        (*stream) << "\n";
      } else if((i + 1) % 10) {
        (*stream) << "  ";
      } else {
        (*stream) << "\n";
        PrintIndent();
      }
    }
  }

private:
  void PrintIndent() const;

  unsigned    indent_;
  QFile       *file;
  QTextStream *stream;
  bool        error_;
};

}

#endif
