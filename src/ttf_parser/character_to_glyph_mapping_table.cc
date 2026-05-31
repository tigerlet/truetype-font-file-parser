#include "stdafx.h"
#include "character_to_glyph_mapping_table.h"
#include "xml_logger.h"

namespace ttf_dll {

EncodingTable::EncodingTable(std::ifstream &fin) {
  FREAD(fin, &format_);
  FREAD(fin, &length_);
  FREAD(fin, &language_);
}

void EncodingTable::DumpTableHeader(XmlLogger &logger) const {
  logger.Println("<format value=\"%u\"/>", format_);
  logger.Println("<length value=\"%u\"/>", length_);
  logger.Println("<language value=\"%u\"/>", language_);
}

void EncodingRecord::LoadRecord(std::ifstream &fin) {
  FREAD(fin, &platform_id_);
  FREAD(fin, &encoding_id_);
  FREAD(fin, &offset_);
}

void EncodingRecord::DumpRecord(XmlLogger &logger) const {
  logger.Println("<encodingRecord>");
  logger.IncreaseIndent();
  logger.Println("<platformID value=\"%u\"/>", platform_id_);
  logger.Println("<encodingID value=\"%u\"/>", encoding_id_);
  logger.Println("<offset value=\"%lu\"/>", offset_);
  logger.DecreaseIndent();
  logger.Println("</encodingRecord>");
}

void EncodingRecord::LoadEncodingTable(std::ifstream &fin, const std::streampos base) {
  fin.seekg(base);
  fin.seekg(offset_, std::ios::cur);
  UShort format = 0;
  FREAD(fin, &format);
  fin.seekg(-(int)sizeof(format), std::ios::cur);
  switch (format) {
    case kByteEncodingTable: {
      encoding_table_ = new ByteEncodingTable(fin);
      break;
    }
    case kHighByteMappingThroughTable: {
      encoding_table_ = new HighByteMappingThroughTable(fin);
      break;
    }
    case kSegmentMappingToDeltaValues: {
      encoding_table_ = new SegmentMappingToDeltaValues(fin);
      break;
    }
    case kTrimmedTableMapping: {
      encoding_table_ = new TrimmedTableMapping(fin);
      break;
    }
    default: {
      encoding_table_ = nullptr;
    }
  }
}

CharacterToGlyphIndexMappingTable::CharacterToGlyphIndexMappingTable(
    const TrueTypeFont& ttf)
    : TtfSubtable(ttf),
      version_(0),
      num_tables_(0),
      encoding_records_(nullptr) {}
  
void CharacterToGlyphIndexMappingTable::Init(
    const TableRecordEntry *entry, std::ifstream &fin) {
  if (!entry) {
    return;
  }
  fin.seekg(entry->offset(), std::ios::beg);
  std::streampos base = fin.tellg();
  FREAD(fin, &version_);
  FREAD(fin, &num_tables_);
  if (num_tables_ == 0) return;
  encoding_records_ = new EncodingRecord[num_tables_];
  EncodingRecord *record = encoding_records_;
  for (int i = 0; i < num_tables_; ++i, ++record) {
    record->LoadRecord(fin);
  }
  record = encoding_records_;
  for (int i = 0; i < num_tables_; ++i, ++record) {
    record->LoadEncodingTable(fin, base);
  }
}

EncodingTable* CharacterToGlyphIndexMappingTable::GetEncodingTable(
    UShort platform_id,
    UShort encoding_id) const {
  EncodingTable* t = nullptr;
  for (int i = 0; i < num_tables_; ++i) {
    if (encoding_records_[i].platform_id() == platform_id &&
        encoding_records_[i].encoding_id() == encoding_id) {
      t = encoding_records_[i].encoding_table();
    }
  }
  return t;
}

GlyphId CharacterToGlyphIndexMappingTable::GetGlyphIndex(
    UShort platform_id,
    UShort encoding_id,
    UShort ch) const {
  GlyphId glyph_index = 0;
  EncodingTable* encoding_table = GetEncodingTable(platform_id, encoding_id);
  if (encoding_table) {
    glyph_index = encoding_table->GetGlyphIndex(ch);
  }
  return glyph_index;
}

ByteEncodingTable::ByteEncodingTable(std::ifstream &fin) : EncodingTable(fin) {
  FREAD_N(fin, glyph_id_array_, 256);
}

GlyphId ByteEncodingTable::GetGlyphIndex(UShort ch) const {
  if (ch >= 256) return 0;
  return glyph_id_array_[ch];
}

HighByteMappingThroughTable::HighByteMappingThroughTable(std::ifstream &fin)
  : EncodingTable(fin) {
  FREAD_N(fin, subheader_keys_, 256);
  UShort var_len = (length_ >> 1) - 259;
  subheaders_and_glyph_id_array = new UShort[var_len];
  FREAD_N(fin, subheaders_and_glyph_id_array, var_len);
}

#define HIBYTE(w) ((Byte)((w) >> 8))
#define LOBYTE(w) ((Byte)(w))
GlyphId HighByteMappingThroughTable::GetGlyphIndex(UShort ch) const {
  Byte high_byte = HIBYTE(ch), low_byte = LOBYTE(ch);
  Subheader *subheader_ptr = (Subheader*)(subheaders_and_glyph_id_array
                                          + subheader_keys_[high_byte]);
  bool single_byte_char = (subheader_keys_[high_byte] == 0);
  UShort index = (single_byte_char ? high_byte : low_byte)
                   - subheader_ptr->first_code;
  if (index >= 0 && index < subheader_ptr->entry_count) {
    UShort glyph_index = *(&subheader_ptr->id_range_offset
                           + (subheader_ptr->id_range_offset >> 1) + index);
    return glyph_index ? (glyph_index + subheader_ptr->id_delta) % 65536 : 0;
  }
  return 0;
}
#undef HIBYTE
#undef LOBYTE

SegmentMappingToDeltaValues::SegmentMappingToDeltaValues(std::ifstream &fin)
  : EncodingTable(fin) {
  FREAD(fin, &seg_countx2_);
  FREAD(fin, &search_range_);
  FREAD(fin, &entry_selector_);
  FREAD(fin, &range_shift_);
  UShort seg_count = seg_countx2_ >> 1;
  if (seg_count == 0) {
    end_count_ = nullptr;
    start_count_ = nullptr;
    id_delta_ = nullptr;
    id_range_offset_and_glyph_id_array_ = nullptr;
    return;
  }

  end_count_ = new UShort[seg_count];
  FREAD_N(fin, end_count_, seg_count);
  FREAD(fin, &reserved_pad_);
  start_count_ = new UShort[seg_count];
  FREAD_N(fin, start_count_, seg_count);
  id_delta_ = new Short[seg_count];
  FREAD_N(fin, id_delta_, seg_count);
  int var_len = (int)seg_count + ((int)(length_ >> 1) - (8 + (seg_countx2_ << 1)));
  if (var_len < 0) var_len = 0;
  id_range_offset_and_glyph_id_array_ = new UShort[var_len];
  FREAD_N(fin, id_range_offset_and_glyph_id_array_, var_len);
}

GlyphId SegmentMappingToDeltaValues::GetGlyphIndex(UShort ch) const {
  if (!end_count_) return 0;
  UShort seg_count = seg_countx2_ >> 1;
  int i = 0;
  while (i < seg_count && end_count_[i] != 0xFFFF && end_count_[i] < ch) {
    ++i;
  }
  if (i >= seg_count) return 0;
  GlyphId glyph_index = 0;
  if (start_count_[i] <= ch) {
    if (id_range_offset_and_glyph_id_array_[i]) {
      glyph_index = *((id_range_offset_and_glyph_id_array_[i] >> 1)
                      + (ch - start_count_[i])
                      + &id_range_offset_and_glyph_id_array_[i]);
    } else {
      glyph_index = (id_delta_[i] + ch) % 65536;
    }
  }
  return glyph_index;
}

TrimmedTableMapping::TrimmedTableMapping(std::ifstream &fin) : EncodingTable(fin) {
  FREAD(fin, &first_code_);
  FREAD(fin, &entry_count_);
  glyph_id_array_ = new UShort[entry_count_];
  FREAD_N(fin, glyph_id_array_, entry_count_);
}

GlyphId TrimmedTableMapping::GetGlyphIndex(UShort ch) const {
  UShort index = ch - first_code_;
  return (index >= 0 && index < entry_count_) ? glyph_id_array_[index] : 0;
}

void CharacterToGlyphIndexMappingTable::DumpInfo(XmlLogger &logger) const {
  logger.Println("<cmap tableVersion=\"0x%08x\" "
                 "numberOfEncodingTables=\"%d\">",
                 version_, num_tables_);
  logger.IncreaseIndent();
  for (int i = 0; i < num_tables_; ++i) {
    encoding_records_[i].DumpRecord(logger);
  }
  for (int i = 0; i < num_tables_; ++i) {
    encoding_records_[i].encoding_table()->DumpInfo(logger);
  }
  logger.DecreaseIndent();
  logger.Println("</cmap>");
}

void ByteEncodingTable::DumpInfo(XmlLogger &logger) const {
  logger.Println("<cmap_format_0>");
  logger.IncreaseIndent();
  DumpTableHeader(logger);
  logger.Println("<glyphIdArray>");
  logger.PrintArray<Byte>(glyph_id_array_, 256, "%8u");
  logger.Println("</glyphIdArray>");
  logger.DecreaseIndent();
  logger.Println("</cmap_format_0>");
}

void HighByteMappingThroughTable::DumpInfo(XmlLogger &logger) const {
  logger.Println("<cmap_format_2>");
  logger.IncreaseIndent();
  DumpTableHeader(logger);
  logger.PrintArray<UShort>(subheader_keys_, 256, "%8u");
  logger.DecreaseIndent();
  logger.Println("</cmap_format_2>");
}

void SegmentMappingToDeltaValues::DumpInfo(XmlLogger &logger) const {
  logger.Println("<cmap_format_4>");
  logger.IncreaseIndent();
  DumpTableHeader(logger);
  logger.Println("<segCountX2 value=\"%u\"/>", seg_countx2_);
  logger.Println("<searchRange value=\"%u\"/>", search_range_);
  logger.Println("<entrySelector value=\"%u\"/>", entry_selector_);
  logger.Println("<rangeShift value=\"%u\"/>", range_shift_);

  logger.Println("<endCount>");
  logger.PrintArray<UShort>(end_count_, (seg_countx2_ >> 1), "%8u");
  logger.Println("</endCount>");

  logger.Println("<reservedPad value=\"%u\"/>", reserved_pad_);

  logger.Println("<startCount>");
  logger.PrintArray<UShort>(start_count_, (seg_countx2_ >> 1), "%8u");
  logger.Println("</startCount>");

  logger.Println("<idDelta>");
  logger.PrintArray<Short>(id_delta_, (seg_countx2_ >> 1), "%8d");
  logger.Println("</idDelta>");

  logger.Println("<idRangeOffset>");
  logger.PrintArray<UShort>(id_range_offset_and_glyph_id_array_,
                            (seg_countx2_ >> 1), "%8u");
  logger.Println("</idRangeOffset>");

  logger.Println("<glyphIdArray>");
  logger.Println("</glyphIdArray>");

  logger.DecreaseIndent();
  logger.Println("</cmap_format_4>");
}

void TrimmedTableMapping::DumpInfo(XmlLogger &logger) const {
  logger.Println("<cmap_format_6>");
  logger.IncreaseIndent();
  DumpTableHeader(logger);
  logger.Println("<firstCode value=\"%u\"/>", first_code_);
  logger.Println("<entryCount value=\"%u\"/>", entry_count_);
  logger.PrintArray<UShort>(glyph_id_array_, entry_count_, "%8u");
  logger.DecreaseIndent();
  logger.Println("</cmap_format_6>");
}

}
