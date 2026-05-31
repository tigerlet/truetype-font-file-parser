DESIGN_SPEC_EN.md
TTF Parser - Qt Version Design Specification
1. Project Overview
1.1 Background
This project is a TrueType Font (TTF) parser based on the Qt framework, providing a cross-platform graphical interface for viewing and analyzing TTF font file structures and glyph data.

1.2 Project Positioning
Target Users: Font designers, developers, font researchers
Core Value: Provides an intuitive TTF file viewing and analysis tool, supporting glyph visualization, character mapping query, and XML export
1.3 Main Features
Feature Module	Description
Open TTF File	Supports opening local TTF font files
Glyph Navigation	Browse all glyphs via slider or input box
Character Mapping	Find glyph by character input
Control Points Display	Show Bézier curve control points of glyphs
XML Export	Export font information to XML format
2. Overall Framework Design
2.1 Architecture Design
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
2.2 Module Division
Module	Directory	Responsibility
UI Layer	src/ui/	GUI display and user interaction
Core Parser Layer	src/ttf_parser/	TTF file parsing and data management
Utility Layer	src/ttf_parser/	Helper utilities like memory stream, XML logger
2.3 Data Flow
TTF File → TrueTypeFont::Open() → Parse OffsetTable → Parse Subtables → Load Glyph Data → Render Display
3. Software Design Specification
3.1 Class Structure Design
3.1.1 Core Class Relationship Diagram
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
3.1.2 Class Responsibility Description
Class Name	Responsibility	Key Fields
TrueTypeFont	Main entry for TTF file parsing, manages all subtables	offset_table_, cmap_, head_, glyf_
OffsetTable	Parses TTF file header, manages table records	sfnt_version_, table_record_entries_
CharacterToGlyphIndexMappingTable	Character to glyph index mapping	encoding_records_
FontHeader	Font global metrics information	x_min_, y_min_, x_max_, y_max_, loca_format_
MaximumProfile	Font maximum outline information	num_glyphs_, max_points_, max_contours_
IndexToLocation	Glyph position index table	offsets_, loca_format_
GlyphData	Glyph data loading and parsing	data_, glyph_
Glyph	Single glyph data	num_contours_, coordinates_, flags_
HorizontalHeader	Horizontal layout metrics header	ascender_, descender_, num_hmetrics_
HorizontalMetrics	Horizontal metrics data	hmetrics, left_side_bearings_
Naming_Table	Font name and copyright information	name_records_
Os2AndWindowsMetrics	OS/2 and Windows metrics	panose_, us_weight_class_
XmlLogger	XML format log output	file, stream
MemStream	Memory stream reader	start_, cursor_, end_
4. Module Functions and API Interface Specification
4.1 UI Layer Module
4.1.1 MainWindow Class
File Location: src/ui/main_window.h

Function Description: Main window class responsible for overall UI layout and user interaction

Public Methods: