#ifndef GLYPH_DATA_H
#define GLYPH_DATA_H
#include "type.h"
#include "ttf_table.h"
#include "index_to_location.h"
#include "mem_stream.h"
#include <QPainterPath>
#include <QPointF>
#include <QTransform>

namespace ttf_dll {

class GlyphData;

class Glyph {
  friend class GlyphData;
public:
  Glyph();
  void Init(UShort num_contours, UShort num_instructions, UShort num_points);
  void Destroy();
  void Reset();
  void GlyphToPath(QPainterPath &path) const;
  void CountPointNum(int *all_pt_num, int *off_pt_num) const;
  void OutputPoints(QPointF *all_pt, int *off_pt) const;
  GlyphId glyph_index() const { return glyph_index_; }
  Short num_contours() const { return num_contours_; }
  FWord x_min() const { return x_min_; };
  FWord y_min() const { return y_min_; };
  FWord x_max() const { return x_max_; };
  FWord y_max() const { return y_max_; };
  bool IsValid() const;
  Short num_points() const { return num_points_; }
  const UShort* end_contours() const { return end_contours_; }
  const QPointF* coordinates() const { return coordinates_; }
  Byte flag_at(int index) const;

private:
  bool IsSimpleGlyph() const { return num_contours_ > 0; }
  bool IsCompositeGlyph() const { return num_contours_ == -1; }
  
  GlyphId           glyph_index_;
  Short             num_contours_;
  FWord             x_min_;
  FWord             y_min_;
  FWord             x_max_;
  FWord             y_max_;
  UShort            *end_contours_;
  UShort            num_instructions_;
  Byte              *instructions_;
  Byte              *flags_;
  QPointF           *coordinates_;
  UShort            num_points_;
};

class GlyphData : public TtfSubtable {
public:
  explicit GlyphData(const TrueTypeFont &ttf);
  void Init(const TableRecordEntry *entry, std::ifstream &fin);
  void Destroy();
  void DumpInfo(XmlLogger &logger) const;
  const Glyph &LoadGlyph(GlyphId glyph_index);

private:
  void LoadSubglyph(GlyphId glyph_index, const QTransform &mtx = QTransform(), UShort depth = 0);
  void LoadSimpleGlyph(MemStream &msm);
  void LoadCompositeGlyph(MemStream &msm, const QTransform &mtx, UShort depth);
  void ReadFlags(MemStream &msm, size_t num_points, Byte *ptr);
  void ReadCoordinates(MemStream &msm, bool read_x, QPointF *ptr);
  Byte              *data_;
  size_t            length_;
  Glyph             glyph_;
  Glyph             subglyph_;
  bool              root_;
};

}

#endif
