#ifndef POST_SCRIPT_H
#define POST_SCRIPT_H

#include "type.h"
#include "ttf_table.h"

namespace ttf_dll {

class Post_Script {
private:
  Fixed  format_type_;
  Fixed  italic_angle_;
  FWord  underline_position_;
  FWord  underline_thickness_;
  ULong  is_fixed_pitch_;
  ULong  min_mem_type42_;
  ULong  max_mem_type42_;
  ULong  min_mem_type1_;
  ULong  max_mem_type1_;
};

}

#endif
