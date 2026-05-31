#include "glyph_data.h"
#include "true_type_font.h"
#include <algorithm>
#include "xml_logger.h"

namespace ttf_dll {

#define BIT(n) (1 << (n))

enum SimpleGlyphDescriptionFlag {
  kOnCurve              = BIT(0),
  kXShortVector         = BIT(1),
  kYShortVector         = BIT(2),
  kRepeat               = BIT(3),
  kThisXIsSame          = BIT(4),
  kThisYIsSame          = BIT(5)
};

enum CompositeGlyphDescriptionFlag {
  kArg1And2AreWords           = BIT(0),
  kArgsAreXyValues            = BIT(1),
  kRoundXyToGrid              = BIT(2),
  kWeHaveAScale               = BIT(3),
  kMoreComponents             = BIT(5),
  kWeHaveAnXAndYScale         = BIT(6),
  kWeHaveATwoByTwo            = BIT(7),
  kWeHaveInstructions         = BIT(8),
  kUseMyMetrics               = BIT(9),
  kOverlapCompound            = BIT(10),
  kScaledComponentOffset      = BIT(11),
  kUnscaledComponentOffset    = BIT(12)
};

static void AddQuadraticBezier(const QPointF &q0, const QPointF &q1, const QPointF &q2, QPainterPath &path);

GlyphData::GlyphData(const TrueTypeFont &ttf)
    : TtfSubtable (ttf),
      data_(nullptr),
      length_(0),
      root_(true) {}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
void GlyphData::Init(const TableRecordEntry *entry, std::ifstream &fin) {
  if (!entry) {
    return;
  }
  fin.seekg(entry->offset(), std::ios::beg);
  length_ = entry->length();
  if (length_ == 0) {
    return;
  }
  data_ = new (std::nothrow) Byte[length_];
  if (!data_) {
    return;
  }
  fin.read((char*)data_, length_);

  const MaximumProfile &maxp = ttf_.maxp();
  UShort mc = MAX(maxp.max_contours(), maxp.max_composite_contours());
  UShort mi = maxp.max_size_of_instructions();
  UShort mp = MAX(maxp.max_points(), maxp.max_composite_points());
  if (mc == 0) mc = 1;
  if (mp == 0) mp = 1;
  glyph_.Init(mc, mi, mp);
}
#undef MAX

void GlyphData::Destroy() {
  DEL_A(data_);
  data_ = nullptr;
  length_ = 0;
  glyph_.Destroy();
  root_ = true;
}

const Glyph &GlyphData::LoadGlyph(GlyphId glyph_index) {
  glyph_.Reset();
  subglyph_ = glyph_;
  root_ = true;
  
  if (glyph_index >= ttf_.maxp().num_glyphs()) {
    return glyph_;
  }
  
  LoadSubglyph(glyph_index);
  return glyph_;
}

void GlyphData::DumpInfo(XmlLogger &logger) const {
  logger.Println("<glyf>");
  logger.Println("</glyf>");
}

void GlyphData::LoadSubglyph(GlyphId glyph_index, const QTransform &mtx, UShort depth) {
  if (!data_) {
    return;
  }
  if (depth > 32) {
    return;
  }
  ULong offset = 0, length = 0;
  ttf_.loca().GetGlyphOffsetAndLength(glyph_index, &offset, &length);
  
  if (!length) {
    return;
  }
  if (offset + length > length_) {
    return;
  }

  MemStream msm((char*)data_ + offset, length);

  subglyph_.glyph_index_ = glyph_index;
  MREAD(msm, &subglyph_.num_contours_);
  
  if (subglyph_.IsSimpleGlyph()) {
    if (subglyph_.num_contours_ > ttf_.maxp().max_contours()) {
      return;
    }
  } else if (subglyph_.IsCompositeGlyph()) {
    if (static_cast<UShort>(-subglyph_.num_contours_) > ttf_.maxp().max_composite_contours()) {
      return;
    }
  }
  
  MREAD(msm, &subglyph_.x_min_);
  MREAD(msm, &subglyph_.y_min_);
  MREAD(msm, &subglyph_.x_max_);
  MREAD(msm, &subglyph_.y_max_);

  if (root_) {
    root_ = false;
    glyph_.glyph_index_ = glyph_index;
    glyph_.num_contours_ = 0;
    glyph_.num_points_ = 0;
    glyph_.num_instructions_ = 0;
    glyph_.x_min_ = subglyph_.x_min_;
    glyph_.y_min_ = subglyph_.y_min_;
    glyph_.x_max_ = subglyph_.x_max_;
    glyph_.y_max_ = subglyph_.y_max_;
  }

  if (!glyph_.end_contours_ || !glyph_.flags_ || !glyph_.coordinates_) {
    return;
  }

  UShort max_contours = ttf_.maxp().max_contours();
  UShort max_points = ttf_.maxp().max_points();

  if (subglyph_.IsSimpleGlyph()) {
    if (glyph_.num_contours_ + subglyph_.num_contours_ > max_contours) {
      return;
    }
    
    subglyph_.end_contours_ = glyph_.end_contours_ + glyph_.num_contours_;
    subglyph_.instructions_ = glyph_.instructions_ + glyph_.num_instructions_;
    subglyph_.flags_ = glyph_.flags_ + glyph_.num_points_;
    subglyph_.coordinates_ = glyph_.coordinates_ + glyph_.num_points_;
    
    LoadSimpleGlyph(msm);
    
    if (subglyph_.num_points_ == 0) {
      return;
    }
    
    if (glyph_.num_points_ + subglyph_.num_points_ > max_points) {
      return;
    }
    
    for (int i = 0; i < subglyph_.num_contours_; ++i) {
      subglyph_.end_contours_[i] += glyph_.num_points_;
    }
    for (int i = 0; i < subglyph_.num_points_; ++i) {
      subglyph_.coordinates_[i] = mtx.map(subglyph_.coordinates_[i]);
    }
    glyph_.num_points_ += subglyph_.num_points_;
    glyph_.num_contours_ += subglyph_.num_contours_;
  } else if (subglyph_.IsCompositeGlyph()) {
    subglyph_.end_contours_ = glyph_.end_contours_ + glyph_.num_contours_;
    subglyph_.instructions_ = glyph_.instructions_ + glyph_.num_instructions_;
    subglyph_.flags_ = glyph_.flags_ + glyph_.num_points_;
    subglyph_.coordinates_ = glyph_.coordinates_ + glyph_.num_points_;
    
    LoadCompositeGlyph(msm, mtx, depth);
  }
}

void GlyphData::LoadSimpleGlyph(MemStream &msm) {
  if (subglyph_.num_contours_ <= 0) {
    return;
  }
  
  if (!subglyph_.end_contours_ || !subglyph_.flags_ || !subglyph_.coordinates_) {
    return;
  }
  
  MREAD_N(msm, subglyph_.end_contours_, subglyph_.num_contours_);
  MREAD(msm, &subglyph_.num_instructions_);
  if (subglyph_.num_instructions_ > ttf_.maxp().max_size_of_instructions()) {
    subglyph_.num_instructions_ = 0;
  }
  msm.Seek(sizeof(Byte) * subglyph_.num_instructions_);
  
  if (subglyph_.num_contours_ <= 0) {
    subglyph_.num_points_ = 0;
    return;
  }
  
  subglyph_.num_points_ = subglyph_.end_contours_[subglyph_.num_contours_ - 1] + 1;
  UShort max_pts = (ttf_.maxp().max_points() > ttf_.maxp().max_composite_points()) ? ttf_.maxp().max_points() : ttf_.maxp().max_composite_points();
  
  if (subglyph_.num_points_ > max_pts || subglyph_.num_points_ <= 0) {
    subglyph_.num_points_ = 0;
    return;
  }
  
  ReadFlags(msm, subglyph_.num_points_, subglyph_.flags_);
  ReadCoordinates(msm, true, subglyph_.coordinates_);
  ReadCoordinates(msm, false, subglyph_.coordinates_);
}

void GlyphData::LoadCompositeGlyph(MemStream &msm, const QTransform &mtx, UShort depth) {
  if (depth + 1 > ttf_.maxp().max_component_depth()) return;

  UShort      flags;
  GlyphId     glyph_index;
  Short       arg1, arg2;
  Short       x, y;
  F2Dot14     xx, xy, yx, yy;

  do {
    x = y = 0;
    xx = yy = 0x4000;
    xy = yx = 0;
    MREAD(msm, &flags);
    MREAD(msm, &glyph_index);
    if (glyph_index >= ttf_.maxp().num_glyphs()) {
      return;
    }
    arg1 = arg2 = 0;
    if (flags & kArg1And2AreWords) {
      MREAD(msm, &arg1);
      MREAD(msm, &arg2);
    } else {
      UShort arg1and2 = 0;
      MREAD(msm, &arg1and2);
      arg1 = static_cast<Char>(arg1and2 >> 8);
      arg2 = static_cast<Char>(arg1and2);
    }
    if (flags & kArgsAreXyValues) {
      x = arg1;
      y = arg2;
    }
    if (flags & kWeHaveAScale) {
      MREAD(msm, &xx);
      yy = xx;
    } else if (flags & kWeHaveAnXAndYScale) {
      MREAD(msm, &xx);
      MREAD(msm, &yy);
    } else if (flags & kWeHaveATwoByTwo) {
      MREAD(msm, &xx);
      MREAD(msm, &xy);
      MREAD(msm, &yx);
      MREAD(msm, &yy);
    }
    QTransform trans(
              F2Dot14ToFloat(xx),
              F2Dot14ToFloat(xy),
              F2Dot14ToFloat(yx),
              F2Dot14ToFloat(yy),
              x, y);
    QTransform combined = trans * mtx;
    LoadSubglyph(glyph_index, combined, depth + 1);
  } while (flags & kMoreComponents);
  if (flags & kWeHaveInstructions) {
    MREAD(msm, &subglyph_.num_instructions_);
    if (subglyph_.num_instructions_ <= ttf_.maxp().max_size_of_instructions()) {
      MREAD_N(msm, subglyph_.instructions_, subglyph_.num_instructions_);
    }
  }
}

void GlyphData::ReadFlags(MemStream &msm, size_t num_points, Byte *ptr) {
  Byte flag = 0;
  for (size_t i = 0; i < num_points;) {
    MREAD(msm, &flag);
    ptr[i++] = flag;
    if (flag & kRepeat) {
      Byte repeat_num = 0;
      MREAD(msm, &repeat_num);
      while (repeat_num-- > 0 && i < num_points) {
        ptr[i++] = flag;
      }
    }
  }
}

void GlyphData::ReadCoordinates(MemStream &msm, bool read_x, QPointF *ptr) {
  Byte flag = 0;
  Byte SHORT_VECTOR = kXShortVector << (read_x ? 0: 1);
  Byte IS_SAME = kThisXIsSame << (read_x ? 0: 1);
  Short val = 0, last = 0;
  for (int i = 0; i < subglyph_.num_points_; ++i, ++ptr) {
    flag = subglyph_.flags_[i];
    val = 0;
    if (flag & SHORT_VECTOR) {
      Byte bval = 0;
      MREAD(msm, &bval);
      val = bval;
      if (~flag & IS_SAME) {
        val = -val;
      }
    } else {
      if (~flag & IS_SAME) {
        Short sval = 0;
        MREAD(msm, &sval);
        val = sval;
      }
    }
    val += last;
    if (read_x) {
      ptr->rx() = static_cast<float>(val);
    } else {
      ptr->ry() = static_cast<float>(val);
    }
    last = val;
  }
}

void Glyph::Reset() {
  glyph_index_ = 0;
  num_contours_ = 0;
  x_min_ = y_min_ = x_max_ = y_max_ = 0;
  num_instructions_ = 0;
  num_points_ = 0;
}

Glyph::Glyph()
    : glyph_index_(0),
      num_contours_(0),
      x_min_(0),
      y_min_(0),
      x_max_(0),
      y_max_(0),
      end_contours_(nullptr),
      num_instructions_(0),
      instructions_(nullptr),
      flags_(nullptr),
      coordinates_(nullptr),
      num_points_(0) {}

void Glyph::Init(UShort num_contours, UShort num_instructions, UShort num_points) {
  if (end_contours_) delete[] end_contours_;
  if (instructions_) delete[] instructions_;
  if (flags_) delete[] flags_;
  if (coordinates_) delete[] coordinates_;

  end_contours_ = nullptr;
  instructions_ = nullptr;
  flags_ = nullptr;
  coordinates_ = nullptr;

  if (num_contours > 0) {
    end_contours_ = new (std::nothrow) UShort[num_contours];
    if (!end_contours_) return;
  }
  if (num_instructions > 0) {
    instructions_ = new (std::nothrow) Byte[num_instructions];
    if (!instructions_) return;
  }
  if (num_points > 0) {
    flags_ = new (std::nothrow) Byte[num_points];
    if (!flags_) return;
    coordinates_ = new (std::nothrow) QPointF[num_points];
  }
}

bool Glyph::IsValid() const {
  return end_contours_ != nullptr && flags_ != nullptr && coordinates_ != nullptr && num_points_ > 0;
}

Byte Glyph::flag_at(int index) const {
  if (index >= 0 && index < num_points_ && flags_) {
    return flags_[index];
  }
  return 0;
}

void Glyph::Destroy() {
  DEL_A(end_contours_);
  DEL_A(instructions_);
  DEL_A(flags_);
  DEL_A(coordinates_);
}

void Glyph::GlyphToPath(QPainterPath &path) const {
  if (!IsValid() || num_points_ <= 0) {
    return;
  }

  path.setFillRule(Qt::WindingFill);

  QPointF start_point, prev_point, cur_point;
  Byte flag = 0;
  bool new_contour = true;
  int last = 0;
  for (int i = 0, j = 0; i < num_points_ && j < num_contours_; ++i) {
    flag = flags_[i];
    cur_point = coordinates_[i];

    if (new_contour) {
      path.moveTo(cur_point);
      new_contour = false;
      last = end_contours_[j];
      if (last >= num_points_) {
        break;
      }
      if (flags_[last] & kOnCurve) {
        prev_point = coordinates_[last];
      } else {
        if (flag & kOnCurve) {
          prev_point = cur_point;
        } else {
          prev_point = QPointF(
                           (cur_point.x() + coordinates_[last].x()) / 2.0f,
                           (cur_point.y() + coordinates_[last].y()) / 2.0f);
        }
      }
      start_point = prev_point;
    }
    if (flag & kOnCurve) {
      path.lineTo(cur_point);
      prev_point = cur_point;
    } else {
      QPointF next_point;
      if (i == last) {
        next_point = start_point;
      } else if (i + 1 < num_points_) {
        Byte next_flag = flags_[i + 1];
        if (next_flag & kOnCurve) {
          next_point = coordinates_[i + 1];
        } else {
          next_point = QPointF(
                         (cur_point.x() + coordinates_[i + 1].x()) / 2.0f,
                         (cur_point.y() + coordinates_[i + 1].y()) / 2.0f
                       );
        }
      } else {
        next_point = cur_point;
      }
      AddQuadraticBezier(prev_point, cur_point, next_point, path);
      prev_point = next_point;
    }
    if (i == last) {
      path.closeSubpath();
      new_contour = true;
      ++j;
    }
  }
}

void Glyph::CountPointNum(int *all_pt_num, int *off_pt_num) const {
  if (!IsValid() || num_points_ <= 0) {
    *all_pt_num = 0;
    *off_pt_num = 0;
    return;
  }

  int all = 0, off = 0;
  bool new_contour = true;
  Byte flag = 0, prev_flag = 0;
  int last = 0;
  for (int i = 0, j = 0; i < num_points_ && j < num_contours_; ++i, ++all) {
    flag = flags_[i] & kOnCurve;
    if (new_contour) {
      new_contour = false;
      last = end_contours_[j];
      if (last >= num_points_) {
        break;
      }
      prev_flag = flags_[last] & kOnCurve;
    }
    if (!flag) {
      ++off;
      if (!prev_flag) {
        ++all;
      }
    }
    if (i == last) {
      new_contour = true;
      ++j;
    }
    prev_flag = flag;
  }
  *all_pt_num = all;
  *off_pt_num = off;
}

void Glyph::OutputPoints(QPointF *all_pt, int *off_pt) const {
  if (!IsValid() || num_points_ <= 0 || !all_pt || !off_pt) {
    return;
  }

  QPointF prev_point, cur_point;
  Byte prev_flag = 0, flag = 0;
  bool new_contour = true;
  int last = 0;
  QPointF *pt = all_pt;
  for (int i = 0, j = 0; i < num_points_ && j < num_contours_; ++i) {
    flag = flags_[i] & kOnCurve;
    cur_point = coordinates_[i];

    if (new_contour) {
      new_contour = false;
      last = end_contours_[j];
      if (last >= num_points_) {
        break;
      }
      prev_flag = flags_[last] & kOnCurve;
      prev_point = coordinates_[last];
    }

    if (!prev_flag && !flag) {
      *pt++ = QPointF(
                (prev_point.x() + cur_point.x()) / 2.0f,
                (prev_point.y() + cur_point.y()) / 2.0f
              );
    }
    if (!flag) {
      *off_pt++ = static_cast<int>(pt - all_pt);
    }
    *pt++ = prev_point = cur_point;
    prev_flag = flag;
    if (i == last) {
      new_contour = true;
      ++j;
    }
  }
}

static void AddQuadraticBezier(const QPointF &q0, const QPointF &q1, const QPointF &q2, QPainterPath &path) {
  QPointF c1, c2;
  c1.rx() = (q0.x() + 2 * q1.x()) / 3.0f;
  c1.ry() = (q0.y() + 2 * q1.y()) / 3.0f;
  c2.rx() = (2 * q1.x() + q2.x()) / 3.0f;
  c2.ry() = (2 * q1.y() + q2.y()) / 3.0f;
  path.cubicTo(c1, c2, q2);
}

}
