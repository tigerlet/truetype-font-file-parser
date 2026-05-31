#include "stdafx.h"
#include "font_header.h"
#include <ctime>
#include "xml_logger.h"

namespace ttf_dll {

static void LongDateTimeToString(char *buf, size_t buf_size,
                                 LongDateTime time);

FontHeader::FontHeader(const TrueTypeFont &ttf)
    : TtfSubtable(ttf),
      table_version_number_(0),
      font_revision_(0),
      checksum_adjustment_(0),
      magic_number_(0),
      flags_(0),
      units_per_em_(0),
      created_(0),
      modified_(0),
      x_min_(0),
      y_min_(0),
      x_max_(0),
      y_max_(0),
      mac_style_(0),
      lowest_rec_ppem_(0),
      font_direction_hint_(0),
      loca_format_(0),
      glyph_data_format_(0) {}

void FontHeader::Init(const TableRecordEntry *entry, std::ifstream &fin) {
  if (!entry) {
    return;
  }
  fin.seekg(entry->offset(), std::ios::beg);
  FREAD(fin, &table_version_number_);
  FREAD(fin, &font_revision_);
  FREAD(fin, &checksum_adjustment_);
  FREAD(fin, &magic_number_);
  FREAD(fin, &flags_);
  FREAD(fin, &units_per_em_);
  FREAD(fin, &created_);
  FREAD(fin, &modified_);
  FREAD(fin, &x_min_);
  FREAD(fin, &y_min_);
  FREAD(fin, &x_max_);
  FREAD(fin, &y_max_);
  FREAD(fin, &mac_style_);
  FREAD(fin, &lowest_rec_ppem_);
  FREAD(fin, &font_direction_hint_);
  FREAD(fin, &loca_format_);
  FREAD(fin, &glyph_data_format_);
}

void FontHeader::DumpInfo(XmlLogger &logger) const {
  logger.Println("<head>");
  logger.IncreaseIndent();
  logger.Println("<tableVersion value=\"0x%08x\"/>", table_version_number_);
  logger.Println("<fontRevision value=\"0x%08x\"/>", font_revision_);
  logger.Println("<checkSumAdjustment value=\"0x%08x\"/>",
                 checksum_adjustment_);
  logger.Println("<magicNumber value=\"0x%08x\"/>", magic_number_);
  logger.Println("<flags value=\"0x%04x\"/>", flags_);
  logger.Println("<unitsPerEm value=\"%d\"/>", units_per_em_);
  char time[50] = {0};
  LongDateTimeToString(time, 50, created_);
  logger.Println("<created value=\"%s\"/>", time);
  LongDateTimeToString(time, 50, modified_);
  logger.Println("<modified value=\"%s\"/>", time);
  logger.Println("<xMin value=\"%d\"/>", x_min_);
  logger.Println("<yMin value=\"%d\"/>", y_min_);
  logger.Println("<xMax value=\"%d\"/>", x_max_);
  logger.Println("<yMax value=\"%d\"/>", y_max_);
  logger.Println("<macStyle value=\"0x%04x\"/>", mac_style_);
  logger.Println("<lowestRecPPEM value=\"%u\"/>", lowest_rec_ppem_);
  logger.Println("<fontDirectionHint value=\"%d\"/>", font_direction_hint_);
  logger.Println("<indexToLocFormat value=\"%d\"/>", loca_format_);
  logger.Println("<glyphDataFormat value=\"%d\"/>", glyph_data_format_);
  logger.DecreaseIndent();
  logger.Println("</head>");
}

static void LongDateTimeToString(char *buf, size_t buf_size,
                                 LongDateTime time) {
  static const unsigned long kSecondsUntil1970 = 2082844800;
  if(time < kSecondsUntil1970) {
    snprintf(buf, buf_size, "Invalid Time Stamp");
    return;
  }
  tm mac_time = {0};
  mac_time.tm_sec = (int)(time - kSecondsUntil1970);
  mac_time.tm_hour = 0;
  mac_time.tm_mday = 1;
  mac_time.tm_year = 70;
  mktime(&mac_time);
  strftime(buf, buf_size, "%a %b %d %X %Y", &mac_time);
}

}
