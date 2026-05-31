#include "glyph_widget.h"
#include <QPainter>
#include <QPainterPath>
#include <QTransform>
#include <cmath>

enum SimpleGlyphDescriptionFlag {
  kOnCurve      = 0x01,
  kXShortVector = 0x02,
  kYShortVector = 0x04,
  kRepeat       = 0x08,
  kThisXIsSame  = 0x10,
  kThisYIsSame  = 0x20
};

GlyphWidget::GlyphWidget(QWidget* parent)
  : QWidget(parent), font_(nullptr), glyphIndex_(0), showPoints_(false), fillPath_(true) {
  setMinimumSize(400, 400);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setAutoFillBackground(true);
  setAttribute(Qt::WA_OpaquePaintEvent, true);
}

void GlyphWidget::setFont(ttf_dll::TrueTypeFont *font) {
  font_ = font;
  update();
}

void GlyphWidget::setGlyphIndex(ttf_dll::GlyphId glyphIndex) {
  glyphIndex_ = glyphIndex;
  update();
}

void GlyphWidget::setShowPoints(bool showPoints) {
  showPoints_ = showPoints;
  update();
}

void GlyphWidget::setFillPath(bool fill) {
  fillPath_ = fill;
  update();
}

void GlyphWidget::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  painter.fillRect(this->rect(), Qt::white);

  if (!font_ || !font_->is_open()) {
    painter.setPen(Qt::black);
    painter.drawText(this->rect(), Qt::AlignCenter, "No font loaded");
    return;
  }

  if (glyphIndex_ >= font_->maxp().num_glyphs()) {
    painter.setPen(Qt::black);
    painter.drawText(this->rect(), Qt::AlignCenter, "Glyph index out of range");
    return;
  }

  const ttf_dll::Glyph& glyph = font_->GetGlyph(glyphIndex_);

  if (!glyph.IsValid()) {
    painter.setPen(Qt::black);
    painter.drawText(this->rect(), Qt::AlignCenter, "Invalid glyph");
    return;
  }

  ttf_dll::FWord ttf_x_max = glyph.x_max();
  ttf_dll::FWord ttf_y_max = glyph.y_max();
  ttf_dll::FWord ttf_x_min = glyph.x_min();
  ttf_dll::FWord ttf_y_min = glyph.y_min();
  ttf_dll::FWord glyph_width = ttf_x_max - ttf_x_min;
  ttf_dll::FWord glyph_height = ttf_y_max - ttf_y_min;

  if (glyph_width <= 0 || glyph_height <= 0) {
    painter.setPen(Qt::black);
    painter.drawText(this->rect(), Qt::AlignCenter, "Invalid glyph dimensions");
    return;
  }

  QPainterPath path;
  glyph.GlyphToPath(path);

  if (path.isEmpty()) {
    painter.setPen(Qt::black);
    painter.drawText(this->rect(), Qt::AlignCenter, "Path is empty");
    return;
  }

  QRect drawArea = this->rect().adjusted(20, 20, -20, -20);

  if (drawArea.width() <= 0 || drawArea.height() <= 0) {
    painter.setPen(Qt::black);
    painter.drawText(this->rect(), Qt::AlignCenter, "Widget too small");
    return;
  }

  float x_ratio = (float)drawArea.width() / (float)glyph_width;
  float y_ratio = (float)drawArea.height() / (float)glyph_height;
  float scale = qMin(x_ratio, y_ratio);

  if (scale <= 0) {
    painter.setPen(Qt::black);
    painter.drawText(this->rect(), Qt::AlignCenter, "Invalid scale");
    return;
  }

  float scaled_w = (float)glyph_width * scale;
  float scaled_h = (float)glyph_height * scale;
  float offset_x = drawArea.x() + (drawArea.width() - scaled_w) / 2.0f;
  float offset_y = drawArea.y() + (drawArea.height() - scaled_h) / 2.0f;

  QTransform transform;
  transform.translate(offset_x, offset_y + scaled_h);
  transform.scale(scale, -scale);
  transform.translate(-ttf_x_min, -ttf_y_min);

  painter.setPen(QPen(Qt::black, 1.5f));
  painter.setBrush(fillPath_ ? QBrush(Qt::black) : QBrush(Qt::NoBrush));
  painter.setTransform(transform);
  painter.drawPath(path);

  if (showPoints_) {
    QFont font = painter.font();
    font.setPointSize(6);
    painter.setFont(font);
    
    for (int i = 0; i < glyph.num_points(); ++i) {
      QPointF pt = glyph.coordinates()[i];
      ttf_dll::Byte flag = glyph.flag_at(i);
      bool onCurve = (flag & kOnCurve) != 0;

      if (onCurve) {
        painter.setPen(QPen(Qt::red, 1.0f));
        painter.setBrush(Qt::red);
        painter.drawEllipse(pt, 1.5f, 1.5f);
      } else {
        painter.setPen(QPen(Qt::blue, 1.0f));
        painter.setBrush(Qt::blue);
        painter.drawEllipse(pt, 1.5f, 1.5f);
      }
    }
    
    painter.resetTransform();
    painter.setPen(QPen(Qt::black, 1.0f));
    for (int i = 0; i < glyph.num_points(); ++i) {
      QPointF pt = glyph.coordinates()[i];
      QPointF screenPt = transform.map(pt);
      painter.drawText(screenPt.x() + 4, screenPt.y() - 4, QString::number(i));
    }
  }

  painter.resetTransform();
  painter.setBrush(Qt::NoBrush);

  int legendX = drawArea.right() - 120;
  int legendY = drawArea.top() + 10;
  
  painter.setPen(QPen(Qt::gray, 1));
  painter.drawRect(legendX - 5, legendY - 5, 125, 35);
  
  painter.setPen(QPen(Qt::black, 1.0f));
  QFont legendFont = painter.font();
  legendFont.setPointSize(8);
  painter.setFont(legendFont);
  
  painter.setPen(QPen(Qt::red, 1.0f));
  painter.setBrush(Qt::red);
  painter.drawEllipse(legendX + 5, legendY + 8, 4, 4);
  painter.setPen(QPen(Qt::black, 1.0f));
  painter.drawText(legendX + 15, legendY + 12, "On-curve");
  
  painter.setPen(QPen(Qt::blue, 1.0f));
  painter.setBrush(Qt::blue);
  painter.drawEllipse(legendX + 70, legendY + 8, 4, 4);
  painter.setPen(QPen(Qt::black, 1.0f));
  painter.drawText(legendX + 80, legendY + 12, "Off-curve");
  
  painter.setBrush(Qt::NoBrush);

  painter.setPen(QPen(Qt::lightGray, 1));
  painter.drawRect(drawArea);
}
