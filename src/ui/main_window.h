#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include "glyph_widget.h"
#include "ttf_parser/true_type_font.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

private slots:
  void onOpenFile();
  void onDumpXml();
  void onViewChar();
  void onGlyphIndexChanged(int index);
  void onShowPointsToggled(int state);
  void onFillPathToggled(int state);

private:
  void setupUi();
  void enableControls(bool enable);
  void updateGlyphInfo();

  GlyphWidget* glyphWidget_;
  QLineEdit* charEdit_;
  QPushButton* viewButton_;
  QSlider* glyphIndexSlider_;
  QSpinBox* glyphIndexSpinBox_;
  QCheckBox* showPointsCheckBox_;
  QCheckBox* fillPathCheckBox_;
  QLabel* fileNameLabel_;
  QLabel* glyphInfoLabel_;
  QPushButton* openButton_;
  QPushButton* dumpXmlButton_;

  ttf_dll::TrueTypeFont ttf_;
};

#endif
