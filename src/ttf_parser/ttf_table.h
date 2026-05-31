#ifndef TTF_TABLE_H
#define TTF_TABLE_H

#include "type.h"

namespace ttf_dll {

class XmlLogger;

class TableRecordEntry {
public:
  TableRecordEntry();
  void LoadEntry(std::ifstream &fin);
  ULong tag() const { return tag_; }
  ULong checksum() const { return checksum_; }
  ULong offset() const { return offset_; }
  ULong length() const { return length_; }

private:
  ULong  tag_;
  ULong  checksum_;
  ULong  offset_;
  ULong  length_;
};

class OffsetTable {
public:
  OffsetTable() : sfnt_version_(0), num_tables_(0), search_range_(0),
                  entry_selector_(0), range_shift_(0), table_record_entries_(nullptr) {}
  void LoadTable(std::ifstream &fin);
  void Destroy() {
    DEL_A(table_record_entries_);
    table_record_entries_ = nullptr;
    num_tables_ = 0;
  }
  const TableRecordEntry *GetTableEntry(const char *tag_str) const;
  void DumpInfo(XmlLogger &logger) const;
  Fixed sfnt_version() const { return sfnt_version_; }

private:
  Fixed   sfnt_version_;
  UShort  num_tables_;
  UShort  search_range_;
  UShort  entry_selector_;
  UShort  range_shift_;
  TableRecordEntry *table_record_entries_;
};

class TrueTypeFont;
class TtfSubtable {
public:
  explicit TtfSubtable(const TrueTypeFont &ttf) : ttf_(ttf) {};
  virtual void Init(const TableRecordEntry *entry, std::ifstream &fin) = 0;
  virtual void Destroy() {};
  virtual void DumpInfo(XmlLogger &logger) const = 0;

protected:
  const TrueTypeFont &ttf_;
};

}

#endif
