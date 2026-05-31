#ifndef FONT_HEADER_H
#define FONT_HEADER_H

#include "type.h"
#include "ttf_table.h"

namespace ttf_dll {
  
class FontHeader : public TtfSubtable {
public:
  explicit FontHeader(const TrueTypeFont &ttf);
  void Init(const TableRecordEntry *entry, std::ifstream &fin);
  void DumpInfo(XmlLogger &logger) const;
  FWord x_min() const { return x_min_; }
  FWord y_min() const { return y_min_; }
  FWord x_max() const { return x_max_; }
  FWord y_max() const { return y_max_; }
  Short loca_format() const { return loca_format_; }

private:
  Fixed             table_version_number_;
  Fixed             font_revision_;
  ULong             checksum_adjustment_;
  ULong             magic_number_;
  UShort            flags_;
  UShort            units_per_em_;
  LongDateTime      created_;
  LongDateTime      modified_;
  FWord             x_min_;
  FWord             y_min_;
  FWord             x_max_;
  FWord             y_max_;

  UShort            mac_style_;
  UShort            lowest_rec_ppem_;
  Short             font_direction_hint_;
  Short             loca_format_;
  Short             glyph_data_format_;
};

}

#endif
