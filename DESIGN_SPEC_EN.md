# TTF Parser - Qt Version Design Specification

## 1. Project Overview

### 1.1 Background
This project is a TrueType Font (TTF) parser based on the Qt framework, providing a cross-platform graphical interface for viewing and analyzing TTF font file structures and glyph data.

### 1.2 Project Positioning
- **Target Users**: Font designers, developers, font researchers
- **Core Value**: Provides an intuitive TTF file viewing and analysis tool, supporting glyph visualization, character mapping query, and XML export

### 1.3 Main Features
| Feature Module | Description |
|---------------|-------------|
| Open TTF File | Supports opening local TTF font files |
| Glyph Navigation | Browse all glyphs via slider or input box |
| Character Mapping | Find glyph by character input |
| Control Points Display | Show Bézier curve control points of glyphs |
| XML Export | Export font information to XML format |

---

## 2. Overall Framework Design

### 2.1 Architecture Design

```
┌─────────────────────────────────────────────────────────────────┐
│                    Application Layer                            │
│  ┌──────────────────┐    ┌──────────────────┐                   │
│  │   MainWindow     │    │   GlyphWidget    │                   │
│  │  (Main Window UI)│    │   (Glyph Render) │                   │
│  └────────┬─────────┘    └────────┬─────────┘                   │
└───────────┼────────────────────────┼─────────────────────────────┘
            │                        │
            ▼                        ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Business Logic Layer                         │
│                         TrueTypeFont                            │
│                  (Font Parsing Core Class)                      │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                        Data Layer                               │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐   │
│  │   cmap  │ │  head   │ │  maxp   │ │  loca   │ │  glyf   │   │
│  │  char   │ │ header  │ │profile  │ │index    │ │ glyph   │   │
│  │ mapping │ │         │ │         │ │location │ │ data    │   │
│  └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘   │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐   │
│  │  hhea   │ │  hmtx   │ │  name   │ │  os_2   │ │post     │   │
│  │horiz.   │ │horiz.   │ │ naming  │ │win/os2  │ │script   │   │
│  │header   │ │metrics  │ │ table   │ │metrics  │ │         │   │
│  └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Module Division

| Module | Directory | Responsibility |
|--------|-----------|----------------|
| **UI Layer** | `src/ui/` | GUI display and user interaction |
| **Core Parser Layer** | `src/ttf_parser/` | TTF file parsing and data management |
| **Utility Layer** | `src/ttf_parser/` | Helper utilities like memory stream, XML logger |

### 2.3 Data Flow

```
TTF File → TrueTypeFont::Open() → Parse OffsetTable → Parse Subtables → Load Glyph Data → Render Display
```

---

## 3. Software Design Specification

### 3.1 Class Structure Design

#### 3.1.1 Core Class Relationship Diagram

```
TrueTypeFont (Main Class)
    │
    ├── OffsetTable (Offset Table)
    │
    ├── CharacterToGlyphIndexMappingTable (cmap)
    │       └── EncodingTable (Multiple Encoding Formats)
    │               ├── ByteEncodingTable
    │               ├── HighByteMappingThroughTable
    │               ├── SegmentMappingToDeltaValues
    │               └── TrimmedTableMapping
    │
    ├── FontHeader (head)
    ├── MaximumProfile (maxp)
    ├── IndexToLocation (loca)
    ├── GlyphData (glyf)
    │       └── Glyph (Glyph Data)
    ├── HorizontalHeader (hhea)
    ├── HorizontalMetrics (hmtx)
    ├── Naming_Table (name)
    ├── Os2AndWindowsMetrics (os_2)
    └── Post_Script (post)
```

#### 3.1.2 Class Responsibility Description

| Class Name | Responsibility | Key Fields |
|------------|----------------|------------|
| `TrueTypeFont` | Main entry for TTF file parsing, manages all subtables | `offset_table_`, `cmap_`, `head_`, `glyf_` |
| `OffsetTable` | Parses TTF file header, manages table records | `sfnt_version_`, `table_record_entries_` |
| `CharacterToGlyphIndexMappingTable` | Character to glyph index mapping | `encoding_records_` |
| `FontHeader` | Font global metrics information | `x_min_`, `y_min_`, `x_max_`, `y_max_`, `loca_format_` |
| `MaximumProfile` | Font maximum outline information | `num_glyphs_`, `max_points_`, `max_contours_` |
| `IndexToLocation` | Glyph position index table | `offsets_`, `loca_format_` |
| `GlyphData` | Glyph data loading and parsing | `data_`, `glyph_` |
| `Glyph` | Single glyph data | `num_contours_`, `coordinates_`, `flags_` |
| `HorizontalHeader` | Horizontal layout metrics header | `ascender_`, `descender_`, `num_hmetrics_` |
| `HorizontalMetrics` | Horizontal metrics data | `hmetrics`, `left_side_bearings_` |
| `Naming_Table` | Font name and copyright information | `name_records_` |
| `Os2AndWindowsMetrics` | OS/2 and Windows metrics | `panose_`, `us_weight_class_` |
| `XmlLogger` | XML format log output | `file`, `stream` |
| `MemStream` | Memory stream reader | `start_`, `cursor_`, `end_` |

---

## 4. Module Functions and API Interface Specification

### 4.1 UI Layer Module

#### 4.1.1 MainWindow Class

**File Location**: `src/ui/main_window.h`

**Function Description**: Main window class responsible for overall UI layout and user interaction

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `MainWindow(QWidget*)` | Constructor | `parent`: Parent window pointer | - |
| `~MainWindow()` | Destructor | - | - |

**Private Slots**:

| Method Name | Description | Trigger Condition |
|-------------|-------------|------------------|
| `onOpenFile()` | Open TTF file | Click "Open TTF..." button |
| `onDumpXml()` | Export XML | Click "Dump XML" button |
| `onViewChar()` | View character glyph | Click "View" button |
| `onGlyphIndexChanged(int)` | Glyph index changed | Slider or input box changed |
| `onShowPointsToggled(int)` | Toggle control points display | Check "Show Points" checkbox |
| `onFillPathToggled(int)` | Toggle fill mode | Check "Fill" checkbox |

**Member Variables**:

| Variable Name | Type | Description |
|---------------|------|-------------|
| `glyphWidget_` | `GlyphWidget*` | Glyph display widget |
| `charEdit_` | `QLineEdit*` | Character input field |
| `glyphIndexSlider_` | `QSlider*` | Glyph index slider |
| `showPointsCheckBox_` | `QCheckBox*` | Show control points checkbox |
| `ttf_` | `TrueTypeFont` | TTF font parser object |

---

#### 4.1.2 GlyphWidget Class

**File Location**: `src/ui/glyph_widget.h`

**Function Description**: Glyph rendering widget, responsible for drawing glyph data to the interface

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `GlyphWidget(QWidget*)` | Constructor | `parent`: Parent window pointer | - |
| `setFont(TrueTypeFont*)` | Set font object | `font`: TTF font pointer | void |
| `setGlyphIndex(GlyphId)` | Set current glyph index | `glyphIndex`: Glyph ID | void |
| `setShowPoints(bool)` | Set whether to show control points | `showPoints`: Show flag | void |
| `setFillPath(bool)` | Set whether to fill path | `fill`: Fill flag | void |

---

### 4.2 TTF Parser Core Module

#### 4.2.1 TrueTypeFont Class

**File Location**: `src/ttf_parser/true_type_font.h`

**Function Description**: Core class for TTF file parsing, responsible for coordinating loading and management of all subtables

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `TrueTypeFont()` | Constructor | - | - |
| `Open(const QString&)` | Open and parse TTF file | `path`: File path | void |
| `Close()` | Close font file | - | void |
| `DumpTtf(const QString&) const` | Export font info to XML | `path`: Output path | bool |
| `GetGlyph(GlyphId)` | Get specified glyph | `glyph_index`: Glyph index | `const Glyph&` |
| `GlyphInfo(const Glyph&, QString&) const` | Get glyph information | `glyph`: Glyph object, `info`: Output string | void |
| `is_open() const` | Check if font is open | - | bool |

**Subtable Access Methods**:

| Method Name | Return Type | Description |
|-------------|-------------|-------------|
| `cmap() const` | `const CharacterToGlyphIndexMappingTable&` | Character mapping table |
| `head() const` | `const FontHeader&` | Font header |
| `maxp() const` | `const MaximumProfile&` | Maximum profile |
| `loca() const` | `const IndexToLocation&` | Index location |
| `hhea() const` | `const HorizontalHeader&` | Horizontal header |
| `hmtx() const` | `const HorizontalMetrics&` | Horizontal metrics |
| `name() const` | `const Naming_Table&` | Naming table |
| `os_2() const` | `const Os2AndWindowsMetrics&` | OS/2 metrics |
| `glyf() const` | `const GlyphData&` | Glyph data |

---

#### 4.2.2 Glyph Class

**File Location**: `src/ttf_parser/glyph_data.h`

**Function Description**: Single glyph data structure containing outlines, control points, etc.

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `Glyph()` | Constructor | - | - |
| `Init(UShort, UShort, UShort)` | Initialize glyph | `num_contours`, `num_instructions`, `num_points` | void |
| `Destroy()` | Release resources | - | void |
| `Reset()` | Reset state | - | void |
| `GlyphToPath(QPainterPath&) const` | Convert to Qt painter path | `path`: Output path | void |
| `CountPointNum(int*, int*) const` | Count points | `all_pt_num`: Total points, `off_pt_num`: Off-curve points | void |
| `OutputPoints(QPointF*, int*) const` | Output control points | `all_pt`: All points, `off_pt`: Off-curve indices | void |
| `IsValid() const` | Check if glyph is valid | - | bool |

**Property Access Methods**:

| Method Name | Return Type | Description |
|-------------|-------------|-------------|
| `glyph_index() const` | `GlyphId` | Glyph index |
| `num_contours() const` | `Short` | Number of contours |
| `x_min() const` | `FWord` | Minimum X |
| `y_min() const` | `FWord` | Minimum Y |
| `x_max() const` | `FWord` | Maximum X |
| `y_max() const` | `FWord` | Maximum Y |
| `num_points() const` | `Short` | Number of points |
| `end_contours() const` | `const UShort*` | Contour end point indices |
| `coordinates() const` | `const QPointF*` | Coordinate array |
| `flag_at(int) const` | `Byte` | Flag at specified index |

---

#### 4.2.3 GlyphData Class

**File Location**: `src/ttf_parser/glyph_data.h`

**Function Description**: Glyph data table, responsible for loading and parsing glyph data

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `GlyphData(const TrueTypeFont&)` | Constructor | `ttf`: Reference to font object | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | Initialize and load | `entry`: Table record, `fin`: File stream | void |
| `Destroy()` | Release resources | - | void |
| `DumpInfo(XmlLogger&) const` | Output info to XML | `logger`: XML logger | void |
| `LoadGlyph(GlyphId)` | Load specified glyph | `glyph_index`: Glyph index | `const Glyph&` |

---

#### 4.2.4 CharacterToGlyphIndexMappingTable Class

**File Location**: `src/ttf_parser/character_to_glyph_mapping_table.h`

**Function Description**: Character to glyph index mapping table, supports multiple encoding formats

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `CharacterToGlyphIndexMappingTable(const TrueTypeFont&)` | Constructor | `ttf`: Reference to font object | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | Initialize and load | `entry`: Table record, `fin`: File stream | void |
| `Destroy()` | Release resources | - | void |
| `DumpInfo(XmlLogger&) const` | Output info to XML | `logger`: XML logger | void |
| `GetGlyphIndex(UShort, UShort, UShort) const` | Get glyph index | `platform_id`, `encoding_id`, `ch`: Character | `GlyphId` |

**Encoding Format Enumeration**:

| Enum Value | Name | Description |
|------------|------|-------------|
| 0 | `kByteEncodingTable` | Single-byte encoding table |
| 2 | `kHighByteMappingThroughTable` | High-byte mapping table |
| 4 | `kSegmentMappingToDeltaValues` | Segment mapping |
| 6 | `kTrimmedTableMapping` | Trimmed table mapping |
| 8 | `kMixed16BitAnd32BitCoverage` | Mixed 16/32-bit coverage |
| 10 | `kTrimmedArray` | Trimmed array |
| 12 | `kSegmentedCoverage` | Segmented coverage |
| 13 | `kManyToOneRangeMappings` | Many-to-one range mappings |
| 14 | `kUnicodeVariationSequences` | Unicode variation sequences |

---

#### 4.2.5 FontHeader Class

**File Location**: `src/ttf_parser/font_header.h`

**Function Description**: Font header table containing global font metrics

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `FontHeader(const TrueTypeFont&)` | Constructor | `ttf`: Reference to font object | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | Initialize and load | `entry`: Table record, `fin`: File stream | void |
| `DumpInfo(XmlLogger&) const` | Output info to XML | `logger`: XML logger | void |

**Property Access Methods**:

| Method Name | Return Type | Description |
|-------------|-------------|-------------|
| `x_min() const` | `FWord` | Font bounding box minimum X |
| `y_min() const` | `FWord` | Font bounding box minimum Y |
| `x_max() const` | `FWord` | Font bounding box maximum X |
| `y_max() const` | `FWord` | Font bounding box maximum Y |
| `loca_format() const` | `Short` | Loca table format (0=short offset, 1=long offset) |

---

#### 4.2.6 MaximumProfile Class

**File Location**: `src/ttf_parser/maximum_profile.h`

**Function Description**: Maximum profile table recording various maximum values in font

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `MaximumProfile(const TrueTypeFont&)` | Constructor | `ttf`: Reference to font object | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | Initialize and load | `entry`: Table record, `fin`: File stream | void |
| `DumpInfo(XmlLogger&) const` | Output info to XML | `logger`: XML logger | void |

**Property Access Methods**:

| Method Name | Return Type | Description |
|-------------|-------------|-------------|
| `num_glyphs() const` | `UShort` | Number of glyphs |
| `max_points() const` | `UShort` | Maximum points |
| `max_contours() const` | `UShort` | Maximum contours |
| `max_composite_points() const` | `UShort` | Max composite points |
| `max_composite_contours() const` | `UShort` | Max composite contours |
| `max_size_of_instructions() const` | `UShort` | Max instruction size |
| `max_component_depth() const` | `UShort` | Max component nesting depth |

---

#### 4.2.7 IndexToLocation Class

**File Location**: `src/ttf_parser/index_to_location.h`

**Function Description**: Index to location table providing glyph offsets in glyf table

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `IndexToLocation(const TrueTypeFont&)` | Constructor | `ttf`: Reference to font object | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | Initialize and load | `entry`: Table record, `fin`: File stream | void |
| `Destroy()` | Release resources | - | void |
| `DumpInfo(XmlLogger&) const` | Output info to XML | `logger`: XML logger | void |
| `GetGlyphOffsetAndLength(GlyphId, ULong*, ULong*) const` | Get glyph offset and length | `glyph_index`, `offset`, `length` | void |

---

#### 4.2.8 HorizontalHeader Class

**File Location**: `src/ttf_parser/horizontal_header.h`

**Function Description**: Horizontal header table containing horizontal metrics metadata

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `HorizontalHeader(const TrueTypeFont&)` | Constructor | `ttf`: Reference to font object | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | Initialize and load | `entry`: Table record, `fin`: File stream | void |
| `DumpInfo(XmlLogger&) const` | Output info to XML | `logger`: XML logger | void |

**Property Access Methods**:

| Method Name | Return Type | Description |
|-------------|-------------|-------------|
| `num_hmetrics() const` | `UShort` | Number of horizontal metrics |

---

#### 4.2.9 HorizontalMetrics Class

**File Location**: `src/ttf_parser/horizontal_metrics.h`

**Function Description**: Horizontal metrics table containing horizontal metrics for each glyph

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `HorizontalMetrics(const TrueTypeFont&)` | Constructor | `ttf`: Reference to font object | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | Initialize and load | `entry`: Table record, `fin`: File stream | void |
| `Destroy()` | Release resources | - | void |
| `DumpInfo(XmlLogger&) const` | Output info to XML | `logger`: XML logger | void |
| `GetLeftSideBearing(GlyphId) const` | Get left side bearing | `glyph_index`: Glyph index | `FWord` |
| `GetAdvanceWidth(GlyphId) const` | Get advance width | `glyph_index`: Glyph index | `UFword` |

---

#### 4.2.10 Naming_Table Class

**File Location**: `src/ttf_parser/naming_table.h`

**Function Description**: Naming table containing font names, copyright, etc.

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `Naming_Table(const TrueTypeFont&)` | Constructor | `ttf`: Reference to font object | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | Initialize and load | `entry`: Table record, `fin`: File stream | void |
| `Destroy()` | Release resources | - | void |
| `DumpInfo(XmlLogger&) const` | Output info to XML | `logger`: XML logger | void |

**Platform ID Enumeration**:

| Enum Value | Name | Description |
|------------|------|-------------|
| 0 | `kUnicode` | Unicode |
| 1 | `kMacintosh` | Macintosh |
| 2 | `kISO` | ISO |
| 3 | `kWindows` | Windows |
| 4 | `kCustom` | Custom |

---

#### 4.2.11 Os2AndWindowsMetrics Class

**File Location**: `src/ttf_parser/os_2_and_windows_metrics.h`

**Function Description**: OS/2 and Windows metrics table containing detailed font metrics

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `Os2AndWindowsMetrics(const TrueTypeFont&)` | Constructor | `ttf`: Reference to font object | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | Initialize and load | `entry`: Table record, `fin`: File stream | void |
| `DumpInfo(XmlLogger&) const` | Output info to XML | `logger`: XML logger | void |

**Key Fields**:

| Field Name | Type | Description |
|------------|------|-------------|
| `version_` | `UShort` | Version number |
| `us_weight_class_` | `UShort` | Weight class |
| `us_width_class_` | `UShort` | Width class |
| `panose_` | `PANOSE` | PANOSE classification |
| `ul_unicode_range_1_~4_` | `ULong` | Unicode coverage ranges |

---

### 4.3 Utility Module

#### 4.3.1 XmlLogger Class

**File Location**: `src/ttf_parser/xml_logger.h`

**Function Description**: XML format log output utility

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `XmlLogger(const QString&)` | Constructor | `out_path`: Output path | - |
| `~XmlLogger()` | Destructor | - | - |
| `Error() const` | Check for errors | - | bool |
| `IncreaseIndent()` | Increase indentation | - | void |
| `DecreaseIndent()` | Decrease indentation | - | void |
| `Print(const char*, ...)` | Print formatted text | `format`: Format string | void |
| `Println(const char*, ...)` | Print formatted text with newline | `format`: Format string | void |
| `WPrint(const wchar_t*, ...)` | Print wide character text | `format`: Format string | void |
| `WPrintln(const wchar_t*, ...)` | Print wide character text with newline | `format`: Format string | void |

---

#### 4.3.2 MemStream Class

**File Location**: `src/ttf_parser/mem_stream.h`

**Function Description**: Memory stream reader supporting big-endian data reading

**Public Methods**:

| Method Name | Description | Parameters | Return Value |
|-------------|-------------|------------|--------------|
| `MemStream(const void*, size_t)` | Constructor | `m`: Memory start address, `size`: Size | - |
| `~MemStream()` | Destructor | - | - |
| `Read(void*, size_t)` | Read data | `buf`: Buffer, `size`: Size | void |
| `Seek(int)` | Move cursor | `offset`: Offset | void |
| `Tell()` | Get current position | - | int |
| `ReadBigEndian(void*, unsigned)` | Read big-endian data | `buf`: Buffer, `size`: Size | void |
| `ReadBigEndian(void*, unsigned, unsigned)` | Read multiple big-endian data | `buf`, `unit_size`, `unit_num` | void |

**Macro Definitions**:

| Macro Name | Description |
|------------|-------------|
| `MREAD(s, b)` | Read single big-endian data |
| `MREAD_N(s, b, n)` | Read n big-endian data items |

---

### 4.4 Type Definitions

**File Location**: `src/ttf_parser/type.h`

**Function Description**: Unified data type definitions

**Basic Type Mapping**:

| Type Alias | Underlying Type | Description |
|------------|-----------------|-------------|
| `Int8` | `signed char` | 8-bit signed integer |
| `Int16` | `signed short` | 16-bit signed integer |
| `Int32` | `signed int` | 32-bit signed integer |
| `Int64` | `signed long long` | 64-bit signed integer |
| `UInt8` | `unsigned char` | 8-bit unsigned integer |
| `UInt16` | `unsigned short` | 16-bit unsigned integer |
| `UInt32` | `unsigned int` | 32-bit unsigned integer |
| `UInt64` | `unsigned long long` | 64-bit unsigned integer |

**TTF Specific Types**:

| Type Alias | Underlying Type | Description |
|------------|-----------------|-------------|
| `Byte` | `UInt8` | Byte |
| `Char` | `Int8` | Character |
| `UShort` | `UInt16` | Unsigned short |
| `Short` | `Int16` | Signed short |
| `ULong` | `UInt32` | Unsigned long |
| `Long` | `Int32` | Signed long |
| `Fixed` | `UInt32` | Fixed point (16.16) |
| `FWord` | `Short` | Font unit |
| `UFword` | `UShort` | Unsigned font unit |
| `F2Dot14` | `UInt16` | 2.14 fixed point |
| `LongDateTime` | `Int64` | Date/time |
| `GlyphId` | `UInt16` | Glyph ID |
| `Offset` | `UInt16` | Offset |

**Helper Functions**:

| Function Name | Description | Parameters | Return Value |
|---------------|-------------|------------|--------------|
| `F2Dot14ToFloat(F2Dot14)` | Convert F2Dot14 to float | `val`: Fixed point value | `float` |
| `TableRecordTagToULong(const char*)` | Convert tag to ULong | `tag`: 4-character tag | `ULong` |

---

## 5. Compilation and Build

### 5.1 Dependencies

| Dependency | Version Requirement |
|------------|---------------------|
| Qt | 5.15+ or 6.0+ |
| CMake | 3.16+ (optional) |

### 5.2 Build Methods

#### Method 1: Qt Creator

1. Open Qt Creator
2. File → Open File or Project
3. Select `ttf_parser_qt.pro`
4. Select build kit
5. Click Run

#### Method 2: qmake Command Line

```bash
cd qt_ttf_parser
mkdir build
cd build
qmake ../ttf_parser_qt.pro
nmake  # or mingw32-make
```

#### Method 3: CMake

```bash
cd qt_ttf_parser
mkdir build
cd build
cmake ..
cmake --build .
```

---

## 6. Project Directory Structure

```
qt_ttf_parser/
├── CMakeLists.txt              # CMake build configuration
├── ttf_parser_qt.pro           # Qt build configuration
├── README.md                   # Project documentation
├── info.xml                    # Project information
├── .qtcreator/                 # Qt Creator configuration
│   └── ttf_parser_qt.pro.user
├── build_qmake/                # Build output directory
│   └── release/
└── src/                        # Source code directory
    ├── main.cc                 # Application entry point
    ├── ui/                     # UI components
    │   ├── main_window.h/cc    # Main window
    │   └── glyph_widget.h/cc   # Glyph rendering widget
    └── ttf_parser/             # TTF parser core
        ├── type.h              # Type definitions
        ├── ttf_table.h/cc      # Table base class
        ├── true_type_font.h/cc # Main parser class
        ├── glyph_data.h/cc     # Glyph data
        ├── font_header.h/cc    # Font header
        ├── maximum_profile.h/cc # Maximum profile
        ├── index_to_location.h/cc # Index location
        ├── character_to_glyph_mapping_table.h/cc # Character mapping
        ├── horizontal_header.h/cc # Horizontal header
        ├── horizontal_metrics.h/cc # Horizontal metrics
        ├── naming_table.h/cc   # Naming table
        ├── os_2_and_windows_metrics.h/cc # OS/2 metrics
        ├── post_script.h/cc    # PostScript
        ├── mem_stream.h/cc     # Memory stream
        ├── xml_logger.h/cc     # XML logger
        └── stdafx.h/cc         # Precompiled header
```