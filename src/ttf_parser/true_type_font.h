#ifndef TTF_PARSER_H
#define TTF_PARSER_H

#include "ttf_table.h"
#include "maximum_profile.h"
#include "index_to_location.h"
#include "glyph_data.h"
#include "font_header.h"
#include "character_to_glyph_mapping_table.h"
#include "horizontal_header.h"
#include "horizontal_metrics.h"
#include "naming_table.h"
#include "os_2_and_windows_metrics.h"
#include "post_script.h"
#include <QString>

namespace ttf_dll {

class TrueTypeFont {
public:
  TrueTypeFont();
  void Open(const QString &path);
  void Close();
  bool DumpTtf(const QString &path) const;
  const Glyph &GetGlyph(const GlyphId glyph_index) {
    return glyf_.LoadGlyph(glyph_index);
  }
  void GlyphInfo(const Glyph &glyph, QString &info) const;
  const CharacterToGlyphIndexMappingTable &cmap() const { return cmap_; }
  const FontHeader &head() const { return head_; }
  const MaximumProfile &maxp() const { return maxp_; }
  const IndexToLocation &loca() const { return loca_; }
  const HorizontalHeader &hhea() const { return hhea_; }
  const HorizontalMetrics &hmtx() const { return hmtx_; }
  const Naming_Table &name() const { return name_; }
  const Os2AndWindowsMetrics &os_2() const { return os_2_; }
  const GlyphData &glyf() const { return glyf_; }
  bool is_open() const { return is_open_; }

private:
  bool Checksum(std::ifstream &fin) const;
  bool TableChecksum(const char *data, const char *tag, size_t file_size) const;

  OffsetTable offset_table_;
  CharacterToGlyphIndexMappingTable cmap_;
  FontHeader head_;
  MaximumProfile maxp_;
  IndexToLocation loca_;
  HorizontalHeader hhea_;
  HorizontalMetrics hmtx_;
  Naming_Table name_;
  Os2AndWindowsMetrics os_2_;
  GlyphData glyf_;
  bool is_open_;
};

}

#endif
