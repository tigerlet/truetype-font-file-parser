#ifndef HORIZONTAL_HEADER_H
#define HORIZONTAL_HEADER_H
#include "type.h"
#include "ttf_table.h"

namespace ttf_dll {

class HorizontalHeader : public TtfSubtable {
public:
  explicit HorizontalHeader(const TrueTypeFont &ttf);
  void Init(const TableRecordEntry *entry, std::ifstream &fin);
  void DumpInfo(XmlLogger &logger) const;
  UShort num_hmetrics() const { return num_hmetrics_; }

private:
  Fixed     table_version_number_;
  FWord     ascender_;
  FWord     descender_;
  FWord     line_gap_;
  UFword    advance_width_max_;
  FWord     min_left_side_bearing_;
  FWord     min_right_side_bearing_;
  FWord     x_max_extent_;
  Short     caret_slope_rise_;
  Short     caret_slope_run_;
  Short     caret_offset_;
  Short     metric_data_format_;
  UShort    num_hmetrics_;
};

}

#endif
