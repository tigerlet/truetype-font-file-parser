#include "stdafx.h"
#include "naming_table.h"
#include "xml_logger.h"

namespace ttf_dll {

Naming_Table::Naming_Table(const TrueTypeFont &ttf)
    : TtfSubtable(ttf),
      format_(0),
      count_(0),
      offset_(0),
      name_records_(nullptr) {}

void NameRecord::LoadRecord(std::ifstream &fin) {
  FREAD(fin, &platform_id_);
  FREAD(fin, &encoding_id_);
  FREAD(fin, &language_id_);
  FREAD(fin, &name_id_);
  FREAD(fin, &length_);
  FREAD(fin, &offset_);
}

void NameRecord::ReadString(std::ifstream &fin) {
  string_ = new char[length_ + 1];
  if(DoubleByteString()) {
    IFStreamReadBigEndian(fin, string_, sizeof(char), length_);
    string_[length_] = 0;
  } else {
    fin.read(string_, length_);
    string_[length_] = 0;
  }
}

void Naming_Table::Init(const TableRecordEntry *entry, std::ifstream &fin) {
  if (!entry) {
    return;
  }
  fin.seekg(entry->offset(), std::ios::beg);
  FREAD(fin, &format_);
  FREAD(fin, &count_);
  FREAD(fin, &offset_);
  if (count_ == 0) {
    return;
  }
  name_records_ = new NameRecord[count_];
  for(int i = 0; i < count_; ++i) {
    name_records_[i].LoadRecord(fin);
  }
  std::streampos string_storage = fin.tellg();
  for(int i = 0; i < count_; ++i) {
    fin.seekg(static_cast<std::streamoff>(string_storage) + name_records_[i].string_offset());
    name_records_[i].ReadString(fin);
  }
}

void Naming_Table::DumpInfo(XmlLogger &logger) const {
  logger.Println("<name>");
  logger.IncreaseIndent();
  logger.Println("<format value=\"%u\"/>", format_);
  logger.Println("<count value=\"%u\"/>", count_);
  logger.Println("<offset value=\"%u\"/>", offset_);
  for(int i = 0; i < count_; ++i) {
    name_records_[i].DumpInfo(logger);
  }
  logger.DecreaseIndent();
  logger.Println("</name>");
}

void NameRecord::DumpInfo(XmlLogger &logger) const {
  logger.Println(
      "<nameRecord platformID=\"%u\" encodingID=\"%u\" languageID=\"%u\" "
      "nameID=\"%u\" length=\"%u\" offset=\"%u\">",
      platform_id_, encoding_id_, language_id_,
      name_id_, length_, offset_);
  logger.IncreaseIndent();
  logger.Println("%s", string_);
  logger.DecreaseIndent();
  logger.Println("</nameRecord>");
}

}
