#include "main_window.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QFile>
#include <QTextStream>

static void debugLog(const QString &message) {
  QFile logFile("c:/Users/xqxym/Desktop/ttf_parser-master/qt_ttf_parser/build_clean/main_window_debug.log");
  if (logFile.open(QIODevice::Append | QIODevice::Text)) {
    QTextStream out(&logFile);
    out << message << Qt::endl;
  }
}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), glyphWidget_(nullptr), charEdit_(nullptr),
    viewButton_(nullptr), glyphIndexSlider_(nullptr),
    glyphIndexSpinBox_(nullptr), showPointsCheckBox_(nullptr),
    fillPathCheckBox_(nullptr), fileNameLabel_(nullptr), glyphInfoLabel_(nullptr) {

  debugLog("[DEBUG] MainWindow::MainWindow - START");
  
  setupUi();
  debugLog("[DEBUG] MainWindow::MainWindow - setupUi done");
  
  enableControls(false);
  debugLog("[DEBUG] MainWindow::MainWindow - enableControls done");
  
  setWindowTitle("TTF Parser - Qt");
  resize(800, 700);
  
  debugLog("[DEBUG] MainWindow::MainWindow - END");
}

MainWindow::~MainWindow() {
  ttf_.Close();
}

void MainWindow::setupUi() {
  QWidget* centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

  QHBoxLayout* topLayout = new QHBoxLayout();
  
  QGroupBox* fileGroup = new QGroupBox("File", this);
  QHBoxLayout* fileLayout = new QHBoxLayout(fileGroup);
  openButton_ = new QPushButton("Open TTF...", this);
  fileNameLabel_ = new QLabel("No file selected", this);
  fileNameLabel_->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  fileNameLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  dumpXmlButton_ = new QPushButton("Dump XML", this);
  fileLayout->addWidget(openButton_);
  fileLayout->addWidget(fileNameLabel_);
  fileLayout->addWidget(dumpXmlButton_);
  topLayout->addWidget(fileGroup);

  QGroupBox* charGroup = new QGroupBox("Character", this);
  QHBoxLayout* charLayout = new QHBoxLayout(charGroup);
  charEdit_ = new QLineEdit(this);
  charEdit_->setMaxLength(1);
  viewButton_ = new QPushButton("View", this);
  charLayout->addWidget(new QLabel("Character:", this));
  charLayout->addWidget(charEdit_);
  charLayout->addWidget(viewButton_);
  topLayout->addWidget(charGroup);
  
  mainLayout->addLayout(topLayout);

  QGroupBox* glyphGroup = new QGroupBox("Glyph", this);
  QHBoxLayout* glyphLayout = new QHBoxLayout(glyphGroup);
  glyphIndexSlider_ = new QSlider(Qt::Horizontal, this);
  glyphIndexSpinBox_ = new QSpinBox(this);
  glyphIndexSpinBox_->setMaximumWidth(80);
  showPointsCheckBox_ = new QCheckBox("Show Points", this);
  fillPathCheckBox_ = new QCheckBox("Fill Path", this);
  fillPathCheckBox_->setChecked(true);
  glyphLayout->addWidget(new QLabel("Glyph Index:", this));
  glyphLayout->addWidget(glyphIndexSlider_);
  glyphLayout->addWidget(glyphIndexSpinBox_);
  glyphLayout->addWidget(showPointsCheckBox_);
  glyphLayout->addWidget(fillPathCheckBox_);
  glyphGroup->setMaximumHeight(80);
  mainLayout->addWidget(glyphGroup);

  QHBoxLayout* bottomLayout = new QHBoxLayout();
  
  QGroupBox* displayGroup = new QGroupBox("Display", this);
  QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);
  glyphWidget_ = new GlyphWidget(this);
  displayLayout->addWidget(glyphWidget_);
  bottomLayout->addWidget(displayGroup);
  bottomLayout->setStretchFactor(displayGroup, 9);

  QGroupBox* infoGroup = new QGroupBox("Glyph Info", this);
  QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
  glyphInfoLabel_ = new QLabel("N/A", this);
  glyphInfoLabel_->setWordWrap(true);
  glyphInfoLabel_->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  glyphInfoLabel_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  infoLayout->addWidget(glyphInfoLabel_);
  bottomLayout->addWidget(infoGroup);
  bottomLayout->setStretchFactor(infoGroup, 1);
  
  mainLayout->addLayout(bottomLayout);

  QMenuBar* menuBar = new QMenuBar(this);
  setMenuBar(menuBar);
  QMenu* fileMenu = menuBar->addMenu("&File");
  QAction* openAction = fileMenu->addAction("&Open...");
  QAction* dumpXmlAction = fileMenu->addAction("&Dump XML");
  fileMenu->addSeparator();
  QAction* exitAction = fileMenu->addAction("E&xit");
  openAction->setShortcut(QKeySequence::Open);
  exitAction->setShortcut(QKeySequence::Quit);

  connect(openButton_, &QPushButton::clicked, this, &MainWindow::onOpenFile);
  connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
  connect(dumpXmlButton_, &QPushButton::clicked, this, &MainWindow::onDumpXml);
  connect(dumpXmlAction, &QAction::triggered, this, &MainWindow::onDumpXml);
  connect(viewButton_, &QPushButton::clicked, this, &MainWindow::onViewChar);
  connect(glyphIndexSlider_, &QSlider::valueChanged, this, &MainWindow::onGlyphIndexChanged);
  connect(glyphIndexSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
          this, &MainWindow::onGlyphIndexChanged);
  connect(showPointsCheckBox_, &QCheckBox::stateChanged,
          this, &MainWindow::onShowPointsToggled);
  connect(fillPathCheckBox_, &QCheckBox::stateChanged,
          this, &MainWindow::onFillPathToggled);
  connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

  statusBar()->showMessage("Ready");
}

void MainWindow::enableControls(bool enable) {
  debugLog("[DEBUG] enableControls - enable=" + QString::number(enable ? 1 : 0));
  
  charEdit_->setEnabled(enable);
  viewButton_->setEnabled(enable);
  glyphIndexSlider_->setEnabled(enable);
  glyphIndexSpinBox_->setEnabled(enable);
  showPointsCheckBox_->setEnabled(enable);
  fillPathCheckBox_->setEnabled(enable);
  dumpXmlButton_->setEnabled(enable);

  if (enable && ttf_.is_open()) {
    debugLog("[DEBUG] enableControls - font is open, getting numGlyphs");
    ttf_dll::GlyphId numGlyphs = ttf_.maxp().num_glyphs();
    debugLog("[DEBUG] enableControls - numGlyphs=" + QString::number(numGlyphs));
    glyphIndexSlider_->setRange(0, numGlyphs - 1);
    glyphIndexSpinBox_->setRange(0, numGlyphs - 1);
    debugLog("[DEBUG] enableControls - calling onGlyphIndexChanged(0)");
    onGlyphIndexChanged(0);
    debugLog("[DEBUG] enableControls - onGlyphIndexChanged(0) done");
  }
  debugLog("[DEBUG] enableControls - END");
}

void MainWindow::updateGlyphInfo() {
  if (!ttf_.is_open()) {
    return;
  }

  const ttf_dll::Glyph& glyph = ttf_.GetGlyph(glyphIndexSpinBox_->value());
  
  if (!glyph.IsValid()) {
    glyphInfoLabel_->setText("Invalid glyph");
    return;
  }
  
  QString info;
  ttf_.GlyphInfo(glyph, info);
  glyphInfoLabel_->setText(info);
  glyphWidget_->setFont(&ttf_);
  glyphWidget_->setGlyphIndex(glyphIndexSpinBox_->value());
  glyphWidget_->setShowPoints(showPointsCheckBox_->isChecked());
  glyphWidget_->setFillPath(fillPathCheckBox_->isChecked());
}

void MainWindow::onOpenFile() {
  debugLog("[DEBUG] onOpenFile - START");
  
  QString fileName = QFileDialog::getOpenFileName(
    this, "Open TTF File", "", "TrueType Font Files (*.ttf)");

  debugLog("[DEBUG] onOpenFile - QFileDialog returned, fileName.isEmpty=" + QString::number(fileName.isEmpty()));

  if (fileName.isEmpty()) {
    debugLog("[DEBUG] onOpenFile - fileName is empty, return");
    return;
  }

  debugLog("[DEBUG] onOpenFile - fileName=" + fileName);
  
  debugLog("[DEBUG] onOpenFile - calling ttf_.Close()");
  ttf_.Close();
  debugLog("[DEBUG] onOpenFile - ttf_.Close() done");
  
  debugLog("[DEBUG] onOpenFile - calling ttf_.Open()");
  ttf_.Open(fileName);
  debugLog("[DEBUG] onOpenFile - ttf_.Open() done, is_open=" + QString::number(ttf_.is_open() ? 1 : 0));

  if (ttf_.is_open()) {
    debugLog("[DEBUG] onOpenFile - file opened, setting fileNameLabel");
    fileNameLabel_->setText(fileName);
    
    debugLog("[DEBUG] onOpenFile - calling enableControls(true)");
    enableControls(true);
    debugLog("[DEBUG] onOpenFile - enableControls(true) done");
    
    statusBar()->showMessage("File loaded successfully");
    debugLog("[DEBUG] onOpenFile - END (success)");
  } else {
    debugLog("[DEBUG] onOpenFile - file open failed");
    QMessageBox::critical(this, "Error", "Failed to open TTF file!");
    fileNameLabel_->setText("No file selected");
    debugLog("[DEBUG] onOpenFile - END (failed)");
  }
}

void MainWindow::onDumpXml() {
  Q_UNUSED(this);
}

void MainWindow::onViewChar() {
  debugLog("[DEBUG] onViewChar - START");
  
  if (!ttf_.is_open()) {
    debugLog("[DEBUG] onViewChar - ttf not open");
    return;
  }

  QString charStr = charEdit_->text();
  debugLog("[DEBUG] onViewChar - charStr='" + charStr + "'");
  
  if (charStr.isEmpty()) {
    debugLog("[DEBUG] onViewChar - charStr is empty");
    return;
  }

  QChar ch = charStr.at(0);
  debugLog("[DEBUG] onViewChar - QChar=" + QString::number(ch.unicode()));
  
  ttf_dll::GlyphId glyphIndex = ttf_.cmap().GetGlyphIndex(3, 1, ch.unicode());
  debugLog("[DEBUG] onViewChar - glyphIndex=" + QString::number(glyphIndex));
  
  glyphIndexSpinBox_->setValue(glyphIndex);
  onGlyphIndexChanged(glyphIndex);
  
  debugLog("[DEBUG] onViewChar - END");
}

void MainWindow::onGlyphIndexChanged(int index) {
  if (glyphIndexSlider_->value() != index) {
    glyphIndexSlider_->setValue(index);
  }
  if (glyphIndexSpinBox_->value() != index) {
    glyphIndexSpinBox_->setValue(index);
  }
  updateGlyphInfo();
}

void MainWindow::onShowPointsToggled(int state) {
  glyphWidget_->setShowPoints(state == Qt::Checked);
}

void MainWindow::onFillPathToggled(int state) {
  glyphWidget_->setFillPath(state == Qt::Checked);
}