#include "true_type_font.h"
#include "xml_logger.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>

namespace ttf_dll {

static inline ULong SwapULong(ULong x);
static ULong CalculateChecksum(const ULong *begin, ULong num_byte, ULong buf_size);

bool TrueTypeFont::TableChecksum(const char *data, const char *tag, size_t file_size) const {
  const TableRecordEntry *entry = offset_table_.GetTableEntry(tag);
  if (!entry || !data) {
    return true;
  }
  ULong offset = entry->offset();
  ULong length = entry->length();
  if (offset + length > file_size) {
    return true;
  }
  bool valid = (entry->checksum() == CalculateChecksum(reinterpret_cast<const ULong*>(data + offset), length, file_size - offset));
  return valid;
}

bool TrueTypeFont::Checksum(std::ifstream &fin) const {
  fin.seekg(0, std::ios::end);
  auto pos = fin.tellg();
  if (pos <= 0) {
    return true;
  }
  size_t length = static_cast<size_t>(pos);
  char *data = new (std::nothrow) char[length];
  if (!data) {
    return true;
  }
  fin.seekg(0, std::ios::beg);
  fin.read(data, length);

  const TableRecordEntry* head_entry = offset_table_.GetTableEntry("head");
  if (!head_entry) {
    delete[] data;
    return true;
  }

  ULong head_offset = head_entry->offset();
  if (head_offset + sizeof(ULong) * 3 <= length) {
    reinterpret_cast<ULong*>(&data[head_offset])[2] = 0UL;
  }

  TableChecksum(data, "cmap", length);
  TableChecksum(data, "head", length);
  TableChecksum(data, "maxp", length);
  TableChecksum(data, "loca", length);
  TableChecksum(data, "hhea", length);
  TableChecksum(data, "hmtx", length);
  TableChecksum(data, "name", length);
  TableChecksum(data, "OS/2", length);
  TableChecksum(data, "glyf", length);

  delete[] data;
  return true;
}

TrueTypeFont::TrueTypeFont()
    : cmap_(*this),
      head_(*this),
      maxp_(*this),
      loca_(*this),
      hhea_(*this),
      hmtx_(*this),
      name_(*this),
      os_2_(*this),
      glyf_(*this),
      is_open_(false) {}

void TrueTypeFont::Open(const QString &path) {
  QFile logFile("ttf_parser_debug.log");
  logFile.open(QIODevice::Append | QIODevice::Text);
  QTextStream out(&logFile);

  out << "[DEBUG] TrueTypeFont::Open: Starting to open file:" << path << "\n";
  
  is_open_ = false;
  std::ifstream fin(path.toLocal8Bit().constData(), std::ios::in | std::ios::binary);
  if (!fin.is_open()) {
    out << "[DEBUG] TrueTypeFont::Open: Failed to open file\n";
    logFile.close();
    return;
  }
  out << "[DEBUG] TrueTypeFont::Open: File opened successfully\n";

  offset_table_.LoadTable(fin);
  out << "[DEBUG] TrueTypeFont::Open: OffsetTable loaded, sfnt_version=" << offset_table_.sfnt_version() << "\n";
  
  if (offset_table_.sfnt_version() != 0x00010000) {
    out << "[DEBUG] TrueTypeFont::Open: Invalid sfnt_version\n";
    logFile.close();
    return;
  }
  out << "[DEBUG] TrueTypeFont::Open: sfnt_version is valid\n";

  const TableRecordEntry* cmap_entry = offset_table_.GetTableEntry("cmap");
  const TableRecordEntry* head_entry = offset_table_.GetTableEntry("head");
  const TableRecordEntry* maxp_entry = offset_table_.GetTableEntry("maxp");
  const TableRecordEntry* loca_entry = offset_table_.GetTableEntry("loca");
  const TableRecordEntry* hhea_entry = offset_table_.GetTableEntry("hhea");
  const TableRecordEntry* hmtx_entry = offset_table_.GetTableEntry("hmtx");
  const TableRecordEntry* name_entry = offset_table_.GetTableEntry("name");
  const TableRecordEntry* os2_entry = offset_table_.GetTableEntry("OS/2");
  const TableRecordEntry* glyf_entry = offset_table_.GetTableEntry("glyf");

  out << "[DEBUG] TrueTypeFont::Open: Table entries - cmap=" << (cmap_entry != nullptr) 
      << ", head=" << (head_entry != nullptr) 
      << ", maxp=" << (maxp_entry != nullptr)
      << ", loca=" << (loca_entry != nullptr)
      << ", hhea=" << (hhea_entry != nullptr)
      << ", hmtx=" << (hmtx_entry != nullptr)
      << ", name=" << (name_entry != nullptr)
      << ", os2=" << (os2_entry != nullptr)
      << ", glyf=" << (glyf_entry != nullptr) << "\n";

  if (!cmap_entry || !head_entry || !maxp_entry || !loca_entry ||
      !hhea_entry || !hmtx_entry || !name_entry || !os2_entry || !glyf_entry) {
    out << "[DEBUG] TrueTypeFont::Open: Missing required table entry\n";
    logFile.close();
    return;
  }
  out << "[DEBUG] TrueTypeFont::Open: All table entries found\n";

  Checksum(fin);
  out << "[DEBUG] TrueTypeFont::Open: Checksum completed\n";
  
  fin.clear();

  out << "[DEBUG] TrueTypeFont::Open: Initializing cmap...\n";
  cmap_.Init(cmap_entry, fin);
  
  out << "[DEBUG] TrueTypeFont::Open: Initializing head...\n";
  head_.Init(head_entry, fin);
  
  out << "[DEBUG] TrueTypeFont::Open: Initializing maxp...\n";
  maxp_.Init(maxp_entry, fin);
  
  out << "[DEBUG] TrueTypeFont::Open: Initializing loca...\n";
  loca_.Init(loca_entry, fin);
  
  out << "[DEBUG] TrueTypeFont::Open: Initializing hhea...\n";
  hhea_.Init(hhea_entry, fin);
  
  out << "[DEBUG] TrueTypeFont::Open: Initializing hmtx...\n";
  hmtx_.Init(hmtx_entry, fin);
  
  out << "[DEBUG] TrueTypeFont::Open: Initializing name...\n";
  name_.Init(name_entry, fin);
  
  out << "[DEBUG] TrueTypeFont::Open: Initializing os_2...\n";
  os_2_.Init(os2_entry, fin);
  
  out << "[DEBUG] TrueTypeFont::Open: Initializing glyf...\n";
  glyf_.Init(glyf_entry, fin);

  fin.close();
  is_open_ = true;
  out << "[DEBUG] TrueTypeFont::Open: All tables initialized successfully\n";
  logFile.close();
  return;
}

void TrueTypeFont::Close() {
  if (!is_open_) {
    return;
  }
  qDebug() << "[DEBUG] TrueTypeFont::Close - Starting";
  offset_table_.Destroy();
  qDebug() << "[DEBUG] TrueTypeFont::Close - offset_table destroyed";
  cmap_.Destroy();
  qDebug() << "[DEBUG] TrueTypeFont::Close - cmap destroyed";
  head_.Destroy();
  qDebug() << "[DEBUG] TrueTypeFont::Close - head destroyed";
  maxp_.Destroy();
  qDebug() << "[DEBUG] TrueTypeFont::Close - maxp destroyed";
  loca_.Destroy();
  qDebug() << "[DEBUG] TrueTypeFont::Close - loca destroyed";
  hhea_.Destroy();
  qDebug() << "[DEBUG] TrueTypeFont::Close - hhea destroyed";
  hmtx_.Destroy();
  qDebug() << "[DEBUG] TrueTypeFont::Close - hmtx destroyed";
  name_.Destroy();
  qDebug() << "[DEBUG] TrueTypeFont::Close - name destroyed";
  os_2_.Destroy();
  qDebug() << "[DEBUG] TrueTypeFont::Close - os_2 destroyed";
  glyf_.Destroy();
  qDebug() << "[DEBUG] TrueTypeFont::Close - glyf destroyed";
  is_open_ = false;
  qDebug() << "[DEBUG] TrueTypeFont::Close - END";
}

bool TrueTypeFont::DumpTtf(const QString &path) const {
  XmlLogger logger(path);
  if (logger.Error()) return false;
  logger.Println("<ttFont>");
  logger.IncreaseIndent();
  offset_table_.DumpInfo(logger);
  cmap_.DumpInfo(logger);
  head_.DumpInfo(logger);
  hhea_.DumpInfo(logger);
  maxp_.DumpInfo(logger);
  os_2_.DumpInfo(logger);
  name_.DumpInfo(logger);
  loca_.DumpInfo(logger);
  glyf_.DumpInfo(logger);
  hmtx_.DumpInfo(logger);
  logger.DecreaseIndent();
  logger.Println("</ttFont>");
  return true;
}

void TrueTypeFont::GlyphInfo(const Glyph &glyph, QString &info) const {
  info += QString("xMin: %1\n").arg(glyph.x_min());
  info += QString("xMax: %1\n").arg(glyph.x_max());
  info += QString("yMin: %1\n").arg(glyph.y_min());
  info += QString("yMax: %1\n").arg(glyph.y_max());
  info += QString("numberOfContours: %1\n").arg(glyph.num_contours());
  info += QString("leftSideBearing: %1\n").arg(hmtx_.GetLeftSideBearing(glyph.glyph_index()));
  info += QString("advanceWidth: %1\n").arg(hmtx_.GetAdvanceWidth(glyph.glyph_index()));
}

static inline ULong SwapULong(ULong x) {
  return ((ULong)((((x) & 0x000000FFU) << 24) |
    (((x) & 0x0000FF00U) << 8)  |
    (((x) & 0x00FF0000U) >> 8)  |
    (((x) & 0xFF000000U) >> 24)));
}

static ULong CalculateChecksum(const ULong *begin, ULong num_byte, ULong buf_size) {
  ULong sum = 0UL;
  ULong aligned_size = ((num_byte + 3) & ~3);
  if (aligned_size > buf_size) {
    aligned_size = buf_size & ~3UL;
  }
  const ULong *end = begin + aligned_size / sizeof(ULong);
  while (begin < end) {
    sum += SwapULong(*begin++);
  }
  return sum;
}

}
