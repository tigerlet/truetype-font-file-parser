#ifndef MAXIMUM_PROFILE_H
#define MAXIMUM_PROFILE_H
#include "type.h"
#include "ttf_table.h"

namespace ttf_dll {

class MaximumProfile : public TtfSubtable {
public:
  explicit MaximumProfile(const TrueTypeFont& ttf);
  void Init(const TableRecordEntry *entry, std::ifstream &fin);
  void DumpInfo(XmlLogger &logger) const;
  UShort num_glyphs() const { return num_glyphs_; }
  UShort max_points() const { return max_points_; }
  UShort max_contours() const { return max_contours_; }
  UShort max_composite_points() const { return max_composite_points_; }
  UShort max_composite_contours() const { return max_composite_contours_; }
  UShort max_size_of_instructions() const {
    return max_size_of_instructions_;
  }
  UShort max_component_depth() const { return max_component_depth_; }

private:
  Fixed   table_version_number_;
  UShort  num_glyphs_;
  UShort  max_points_;
  UShort  max_contours_;
  UShort  max_composite_points_;
  UShort  max_composite_contours_;
  UShort  max_zones_;
  UShort  max_twilight_points_;
  UShort  max_storage_;
  UShort  max_function_defs_;
  UShort  max_instruction_defs_;
  UShort  max_stack_elements_;
  UShort  max_size_of_instructions_;
  UShort  max_component_elements_;
  UShort  max_component_depth_;
};

}

#endif
