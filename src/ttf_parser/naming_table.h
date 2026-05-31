#ifndef NAMING_TABLE_H
#define NAMING_TABLE_H
#include "type.h"
#include "ttf_table.h"

namespace ttf_dll {

enum PlatformID {
  kUnicode          = 0,
  kMacintosh        = 1,
  kISO              = 2,
  kWindows          = 3,
  kCustom           = 4
};

enum WindowsEncodingID {
  kSymbol           = 0,
  kUnicodeBMP       = 1,
  kShiftJIS         = 2,
  kPRC              = 3,
  kBig5             = 4,
  kWansung          = 5,
  kJohab            = 6,
  kUnicodeUCS4      = 10
};

class NameRecord {
public:
  ~NameRecord() {
    DEL_A(string_);
  }
  void LoadRecord(std::ifstream &fin);
  bool DoubleByteString() const {
    return
      platform_id_ == kUnicode ||
      platform_id_ == kWindows &&
      encoding_id_ == kUnicodeBMP;
  }
  void DumpInfo(XmlLogger &logger) const;
  void ReadString(std::ifstream &fin);
  UShort string_offset() const { return offset_; }

private:
  UShort  platform_id_;
  UShort  encoding_id_;
  UShort  language_id_;
  UShort  name_id_;
  UShort  length_;
  UShort  offset_;
  char    *string_;
};

class Naming_Table : public TtfSubtable {
public:
  explicit Naming_Table(const TrueTypeFont &ttf);
  void Init(const TableRecordEntry *entry, std::ifstream &fin);
  void Destroy() {
    DEL_A(name_records_);
  }
  void DumpInfo(XmlLogger &logger) const;

private:
  UShort        format_;
  UShort        count_;
  UShort        offset_;
  NameRecord   *name_records_;
};

struct LanguageTagRecord {
  UShort  length_;
  UShort  offset_;
};

}

#endif
