#ifndef CHARACTER_TO_GLYPH_MAPPING_TABLE_H
#define CHARACTER_TO_GLYPH_MAPPING_TABLE_H
#include "type.h"
#include "ttf_table.h"

namespace ttf_dll {

class EncodingTable {
public:
  explicit EncodingTable(std::ifstream &fin);
  virtual ~EncodingTable() {}
  virtual GlyphId GetGlyphIndex(UShort ch) const = 0;
  void DumpTableHeader(XmlLogger &logger) const;
  virtual void DumpInfo(XmlLogger &logger) const = 0;

protected:
  UShort  format_;
  UShort  length_;
  UShort  language_;
};

class EncodingRecord {
public:
  ~EncodingRecord() {
    DEL(encoding_table_);
  }
  void LoadRecord(std::ifstream &fin);
  void DumpRecord(XmlLogger &logger) const;
  void LoadEncodingTable(std::ifstream &fin, const std::streampos base);
  UShort platform_id() const { return platform_id_; };
  UShort encoding_id() const { return encoding_id_; };
  EncodingTable *encoding_table() const { return encoding_table_; };

private:
  UShort              platform_id_;
  UShort              encoding_id_;
  ULong               offset_;
  EncodingTable       *encoding_table_;
};

class CharacterToGlyphIndexMappingTable : public TtfSubtable {
public:
  explicit CharacterToGlyphIndexMappingTable(const TrueTypeFont& ttf);
  void Init(const TableRecordEntry *entry, std::ifstream &fin);
  void Destroy() {
    DEL_A(encoding_records_);
  }
  void DumpInfo(XmlLogger &logger) const;
  GlyphId GetGlyphIndex(UShort platform_id, UShort encoding_id,
                        UShort ch) const;

private:
  EncodingTable *GetEncodingTable(UShort platform_id,
                                  UShort encoding_id) const;
  UShort              version_;
  UShort              num_tables_;
  EncodingRecord      *encoding_records_;
};

enum EncodingTableFormat {
  kByteEncodingTable                        = 0,
  kHighByteMappingThroughTable              = 2,
  kSegmentMappingToDeltaValues              = 4,
  kTrimmedTableMapping                      = 6,
  kMixed16BitAnd32BitCoverage               = 8,
  kTrimmedArray                             = 10,
  kSegmentedCoverage                        = 12,
  kManyToOneRangeMappings                   = 13,
  kUnicodeVariationSequences                = 14
};

class ByteEncodingTable: public EncodingTable {
public:
  explicit ByteEncodingTable(std::ifstream &fin);
  ~ByteEncodingTable() {}
  GlyphId GetGlyphIndex(UShort ch) const;
  void DumpInfo(XmlLogger &logger) const;

private:
  Byte  glyph_id_array_[256];
};

class HighByteMappingThroughTable: public EncodingTable {
public:
  explicit HighByteMappingThroughTable(std::ifstream &fin);
  ~HighByteMappingThroughTable() {
    DEL_A(subheaders_and_glyph_id_array);
  }
  GlyphId GetGlyphIndex(UShort ch) const;
  void DumpInfo(XmlLogger &logger) const;

private:
  UShort    subheader_keys_[256];
  struct Subheader {
    UShort  first_code;
    UShort  entry_count;
    Short   id_delta;
    UShort  id_range_offset;
  };
  UShort    *subheaders_and_glyph_id_array;
};

class SegmentMappingToDeltaValues: public EncodingTable {
public:
  explicit SegmentMappingToDeltaValues(std::ifstream &fin);
  ~SegmentMappingToDeltaValues() {
    DEL_A(end_count_);
    DEL_A(start_count_);
    DEL_A(id_delta_);
    DEL_A(id_range_offset_and_glyph_id_array_);
  }
  GlyphId GetGlyphIndex(UShort ch) const;
  void DumpInfo(XmlLogger &logger) const;

private:
  UShort  seg_countx2_;
  UShort  search_range_;
  UShort  entry_selector_;
  UShort  range_shift_;
  UShort  *end_count_;
  UShort  reserved_pad_;
  UShort  *start_count_;
  Short   *id_delta_;
  UShort  *id_range_offset_and_glyph_id_array_;
  UShort  glyph_id_array_len;
};

class TrimmedTableMapping: public EncodingTable {
public:
  explicit TrimmedTableMapping(std::ifstream &fin);
  ~TrimmedTableMapping() {
    DEL_A(glyph_id_array_);
  }
  GlyphId GetGlyphIndex(UShort ch) const;
  void DumpInfo(XmlLogger &logger) const;

private:
  UShort  first_code_;
  UShort  entry_count_;
  UShort  *glyph_id_array_;
};

}

#endif
