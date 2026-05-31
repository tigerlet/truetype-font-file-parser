# TTF Parser \- Qt Version Design Specification

# 1\. Project Overview

## 1\.1 Background

This project is a TrueType Font \(TTF\) parser based on the Qt framework, providing a cross\-platform graphical interface for viewing and analyzing TTF font file structures and glyph data\.

## 1\.2 Project Positioning

**Target Users**: Font designers, developers, font researchers

**Core Value**: Provides an intuitive TTF file viewing and analysis tool, supporting glyph visualization, character mapping query, and XML export

## 1\.3 Main Features

|Feature Module|Description|
|---|---|
|Open TTF File|Supports opening local TTF font files|
|Glyph Navigation|Browse all glyphs via slider or input box|
|Character Mapping|Find glyph by character input|
|Control Points Display|Show Bézier curve control points of glyphs|
|XML Export|Export font information to XML format|

# 2\. Overall Framework Design

## 2\.1 Architecture Design

## 2\.2 Module Division

|Module|Directory|Responsibility|
|---|---|---|
|UI Layer|src/ui/|GUI display and user interaction|
|Core Parser Layer|src/ttf\_parser/|TTF file parsing and data management|
|Utility Layer|src/ttf\_parser/|Helper utilities like memory stream, XML logger|

## 2\.3 Data Flow

TTF File → TrueTypeFont::Open\(\) → Parse OffsetTable → Parse Subtables → Load Glyph Data → Render Display

# 3\. Software Design Specification

## 3\.1 Class Structure Design

### 3\.1\.1 Core Class Relationship Diagram

### 3\.1\.2 Class Responsibility Description

|Class Name|Responsibility|Key Fields|
|---|---|---|
|TrueTypeFont|Main entry for TTF file parsing, manages all subtables|offset\_table\_, cmap\_, head\_, glyf\_|
|OffsetTable|Parses TTF file header, manages table records|sfnt\_version\_, table\_record\_entries\_|
|CharacterToGlyphIndexMappingTable|Character to glyph index mapping|encoding\_records\_|
|FontHeader|Font global metrics information|x\_min\_, y\_min\_, x\_max\_, y\_max\_, loca\_format\_|
|MaximumProfile|Font maximum outline information|num\_glyphs\_, max\_points\_, max\_contours\_|
|IndexToLocation|Glyph position index table|offsets\_, loca\_format\_|
|GlyphData|Glyph data loading and parsing|data\_, glyph\_|
|Glyph|Single glyph data|num\_contours\_, coordinates\_, flags\_|
|HorizontalHeader|Horizontal layout metrics header|ascender\_, descender\_, num\_hmetrics\_|
|HorizontalMetrics|Horizontal metrics data|hmetrics, left\_side\_bearings\_|
|Naming\_Table|Font name and copyright information|name\_records\_|
|Os2AndWindowsMetrics|OS/2 and Windows metrics|panose\_, us\_weight\_class\_|
|XmlLogger|XML format log output|file, stream|
|MemStream|Memory stream reader|start\_, cursor\_, end\_|

# 4\. Module Functions and API Interface Specification

## 4\.1 UI Layer Module

### 4\.1\.1 MainWindow Class

**File Location**: src/ui/main\_window\.h

**Function Description**: Main window class responsible for overall UI layout and user interaction

