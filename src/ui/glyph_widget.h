#ifndef GLYPH_WIDGET_H
#define GLYPH_WIDGET_H

#include <QWidget>
#include "ttf_parser/true_type_font.h"

class GlyphWidget : public QWidget {
  Q_OBJECT

public:
  explicit GlyphWidget(QWidget* parent = nullptr);
  void setFont(ttf_dll::TrueTypeFont *font);
  void setGlyphIndex(ttf_dll::GlyphId glyphIndex);
  void setShowPoints(bool showPoints);
  void setFillPath(bool fill);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  ttf_dll::TrueTypeFont *font_;
  ttf_dll::GlyphId glyphIndex_;
  bool showPoints_;
  bool fillPath_;
};

#endif
