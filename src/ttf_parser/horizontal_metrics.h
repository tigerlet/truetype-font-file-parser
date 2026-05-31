#ifndef HORIZONTAL_METRICS_H
#define HORIZONTAL_METRICS_H

#include "type.h"
#include "ttf_table.h"

namespace ttf_dll {

struct LongHorMetric {
  void LoadMetric(std::ifstream &fin);

  UShort    advance_width_;
  Short     left_side_bearing_;
};

class HorizontalMetrics : public TtfSubtable {
public:
  explicit HorizontalMetrics(const TrueTypeFont &ttf);
  void Init(const TableRecordEntry *entry, std::ifstream &fin);
  void Destroy() {
    DEL_A(hmetrics);
    DEL_A(left_side_bearings_);
  }
  void DumpInfo(XmlLogger &logger) const;
  FWord GetLeftSideBearing(GlyphId glyph_index) const;
  UFword GetAdvanceWidth(GlyphId glyph_index) const;

private:
  LongHorMetric   *hmetrics;
  FWord             *left_side_bearings_;
  UShort            num_hmtx_;
  UShort            num_glyphs_;
};

}

#endif
