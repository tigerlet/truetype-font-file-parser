#ifndef INDEX_TO_LOCATION_H
#define INDEX_TO_LOCATION_H
#include "type.h"
#include "ttf_table.h"

namespace ttf_dll {

class IndexToLocation : public TtfSubtable {
public:
  explicit IndexToLocation(const TrueTypeFont &ttf);
  void Init(const TableRecordEntry *entry, std::ifstream &fin);
  void Destroy() {
    if (loca_format_) {
      DEL_T(offsets_, ULong);
    } else {
      DEL_T(offsets_, UShort);
    }
  }
  void DumpInfo(XmlLogger &logger) const;
  void GetGlyphOffsetAndLength(GlyphId glyph_index,
                               ULong *offset, ULong *length) const;

private:
  void    *offsets_;
  UShort  num_glyphs_;
  Short   loca_format_;
};

}

#endif
