# TTF Parser - Qt 版本 设计说明书

## 1. 项目概述

### 1.1 项目背景
本项目是一个基于 Qt 框架的 TrueType 字体文件（TTF）解析器，提供跨平台的图形界面用于查看和分析 TTF 字体文件的结构和字形数据。

### 1.2 项目定位
- **目标用户**：字体设计师、开发人员、字体研究人员
- **核心价值**：提供直观的 TTF 文件查看和分析工具，支持字形可视化、字符映射查询和 XML 导出

### 1.3 主要功能
| 功能模块 | 功能描述 |
|---------|---------|
| TTF 文件打开 | 支持打开本地 TTF 字体文件 |
| 字形浏览 | 通过滑块或输入框浏览所有字形 |
| 字符映射 | 根据字符查找对应的字形 |
| 控制点显示 | 显示字形的贝塞尔曲线控制点 |
| XML 导出 | 将字体信息导出为 XML 格式 |

---

## 2. 总体框架设计

### 2.1 架构设计

```
┌─────────────────────────────────────────────────────────────────┐
│                      应用层 (Application Layer)                  │
│  ┌──────────────────┐    ┌──────────────────┐                   │
│  │   MainWindow     │    │   GlyphWidget    │                   │
│  │  (主窗口UI)       │    │   (字形渲染)      │                   │
│  └────────┬─────────┘    └────────┬─────────┘                   │
└───────────┼────────────────────────┼─────────────────────────────┘
            │                        │
            ▼                        ▼
┌─────────────────────────────────────────────────────────────────┐
│                      业务逻辑层 (Business Layer)                 │
│                         TrueTypeFont                            │
│                    (字体解析核心类)                               │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                      数据层 (Data Layer)                         │
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

### 2.2 模块划分

| 模块 | 目录 | 职责 |
|-----|------|-----|
| **UI层** | `src/ui/` | 图形界面展示和用户交互 |
| **核心解析层** | `src/ttf_parser/` | TTF 文件解析和数据管理 |
| **工具层** | `src/ttf_parser/` | 内存流、XML 日志等辅助工具 |

### 2.3 数据流

```
TTF文件 → TrueTypeFont::Open() → 解析OffsetTable → 解析各子表 → 加载字形数据 → 渲染显示
```

---

## 3. 软件设计说明

### 3.1 类结构设计

#### 3.1.1 核心类关系图

```
TrueTypeFont (主类)
    │
    ├── OffsetTable (偏移量表)
    │
    ├── CharacterToGlyphIndexMappingTable (cmap)
    │       └── EncodingTable (多种编码格式)
    │               ├── ByteEncodingTable
    │               ├── HighByteMappingThroughTable
    │               ├── SegmentMappingToDeltaValues
    │               └── TrimmedTableMapping
    │
    ├── FontHeader (head)
    ├── MaximumProfile (maxp)
    ├── IndexToLocation (loca)
    ├── GlyphData (glyf)
    │       └── Glyph (字形数据)
    ├── HorizontalHeader (hhea)
    ├── HorizontalMetrics (hmtx)
    ├── Naming_Table (name)
    ├── Os2AndWindowsMetrics (os_2)
    └── Post_Script (post)
```

#### 3.1.2 类职责说明

| 类名 | 职责 | 关键字段 |
|-----|------|---------|
| `TrueTypeFont` | TTF 文件解析主入口，管理所有子表 | `offset_table_`, `cmap_`, `head_`, `glyf_` |
| `OffsetTable` | 解析 TTF 文件头，管理表记录 | `sfnt_version_`, `table_record_entries_` |
| `CharacterToGlyphIndexMappingTable` | 字符到字形索引的映射 | `encoding_records_` |
| `FontHeader` | 字体全局度量信息 | `x_min_`, `y_min_`, `x_max_`, `y_max_`, `loca_format_` |
| `MaximumProfile` | 字体最大轮廓信息 | `num_glyphs_`, `max_points_`, `max_contours_` |
| `IndexToLocation` | 字形位置索引表 | `offsets_`, `loca_format_` |
| `GlyphData` | 字形数据加载和解析 | `data_`, `glyph_` |
| `Glyph` | 单个字形数据 | `num_contours_`, `coordinates_`, `flags_` |
| `HorizontalHeader` | 水平布局度量头 | `ascender_`, `descender_`, `num_hmetrics_` |
| `HorizontalMetrics` | 水平度量数据 | `hmetrics`, `left_side_bearings_` |
| `Naming_Table` | 字体名称和版权信息 | `name_records_` |
| `Os2AndWindowsMetrics` | OS/2 和 Windows 度量信息 | `panose_`, `us_weight_class_` |
| `XmlLogger` | XML 格式日志输出 | `file`, `stream` |
| `MemStream` | 内存流读取工具 | `start_`, `cursor_`, `end_` |

---

## 4. 模块功能及 API 接口说明

### 4.1 UI 层模块

#### 4.1.1 MainWindow 类

**文件位置**：`src/ui/main_window.h`

**功能说明**：主窗口类，负责整体 UI 布局和用户交互

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `MainWindow(QWidget*)` | 构造函数 | `parent`: 父窗口指针 | - |
| `~MainWindow()` | 析构函数 | - | - |

**私有槽函数**：

| 方法名 | 功能说明 | 触发条件 |
|-------|---------|---------|
| `onOpenFile()` | 打开 TTF 文件 | 点击"Open TTF..."按钮 |
| `onDumpXml()` | 导出 XML | 点击"Dump XML"按钮 |
| `onViewChar()` | 查看字符对应字形 | 点击"View"按钮 |
| `onGlyphIndexChanged(int)` | 字形索引改变 | 滑块或输入框改变 |
| `onShowPointsToggled(int)` | 切换控制点显示 | 勾选"Show Points"复选框 |
| `onFillPathToggled(int)` | 切换填充模式 | 勾选"Fill"复选框 |

**成员变量**：

| 变量名 | 类型 | 说明 |
|-------|------|------|
| `glyphWidget_` | `GlyphWidget*` | 字形显示控件 |
| `charEdit_` | `QLineEdit*` | 字符输入框 |
| `glyphIndexSlider_` | `QSlider*` | 字形索引滑块 |
| `showPointsCheckBox_` | `QCheckBox*` | 显示控制点复选框 |
| `ttf_` | `TrueTypeFont` | TTF 字体解析对象 |

---

#### 4.1.2 GlyphWidget 类

**文件位置**：`src/ui/glyph_widget.h`

**功能说明**：字形渲染控件，负责将字形数据绘制到界面

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `GlyphWidget(QWidget*)` | 构造函数 | `parent`: 父窗口指针 | - |
| `setFont(TrueTypeFont*)` | 设置字体对象 | `font`: TTF 字体指针 | void |
| `setGlyphIndex(GlyphId)` | 设置当前字形索引 | `glyphIndex`: 字形 ID | void |
| `setShowPoints(bool)` | 设置是否显示控制点 | `showPoints`: 显示标志 | void |
| `setFillPath(bool)` | 设置是否填充路径 | `fill`: 填充标志 | void |

---

### 4.2 TTF 解析核心模块

#### 4.2.1 TrueTypeFont 类

**文件位置**：`src/ttf_parser/true_type_font.h`

**功能说明**：TTF 文件解析的核心类，负责协调所有子表的加载和管理

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `TrueTypeFont()` | 构造函数 | - | - |
| `Open(const QString&)` | 打开并解析 TTF 文件 | `path`: 文件路径 | void |
| `Close()` | 关闭字体文件 | - | void |
| `DumpTtf(const QString&) const` | 导出字体信息到 XML | `path`: 输出路径 | bool |
| `GetGlyph(GlyphId)` | 获取指定字形 | `glyph_index`: 字形索引 | `const Glyph&` |
| `GlyphInfo(const Glyph&, QString&) const` | 获取字形信息 | `glyph`: 字形对象, `info`: 输出字符串 | void |
| `is_open() const` | 检查字体是否已打开 | - | bool |

**子表访问方法**：

| 方法名 | 返回类型 | 说明 |
|-------|---------|------|
| `cmap() const` | `const CharacterToGlyphIndexMappingTable&` | 字符映射表 |
| `head() const` | `const FontHeader&` | 字体头 |
| `maxp() const` | `const MaximumProfile&` | 最大轮廓 |
| `loca() const` | `const IndexToLocation&` | 索引定位 |
| `hhea() const` | `const HorizontalHeader&` | 水平头 |
| `hmtx() const` | `const HorizontalMetrics&` | 水平度量 |
| `name() const` | `const Naming_Table&` | 命名表 |
| `os_2() const` | `const Os2AndWindowsMetrics&` | OS/2 度量 |
| `glyf() const` | `const GlyphData&` | 字形数据 |

---

#### 4.2.2 Glyph 类

**文件位置**：`src/ttf_parser/glyph_data.h`

**功能说明**：单个字形的数据结构，包含轮廓、控制点等信息

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `Glyph()` | 构造函数 | - | - |
| `Init(UShort, UShort, UShort)` | 初始化字形 | `num_contours`, `num_instructions`, `num_points` | void |
| `Destroy()` | 释放资源 | - | void |
| `Reset()` | 重置状态 | - | void |
| `GlyphToPath(QPainterPath&) const` | 转换为 Qt 绘制路径 | `path`: 输出路径 | void |
| `CountPointNum(int*, int*) const` | 统计点数 | `all_pt_num`: 总点数, `off_pt_num`: 非控制点数量 | void |
| `OutputPoints(QPointF*, int*) const` | 输出控制点坐标 | `all_pt`: 所有点, `off_pt`: 非控制点索引 | void |
| `IsValid() const` | 检查字形是否有效 | - | bool |

**属性访问方法**：

| 方法名 | 返回类型 | 说明 |
|-------|---------|------|
| `glyph_index() const` | `GlyphId` | 字形索引 |
| `num_contours() const` | `Short` | 轮廓数量 |
| `x_min() const` | `FWord` | 最小 X |
| `y_min() const` | `FWord` | 最小 Y |
| `x_max() const` | `FWord` | 最大 X |
| `y_max() const` | `FWord` | 最大 Y |
| `num_points() const` | `Short` | 点数 |
| `end_contours() const` | `const UShort*` | 轮廓结束点索引数组 |
| `coordinates() const` | `const QPointF*` | 坐标数组 |
| `flag_at(int) const` | `Byte` | 指定索引的标志位 |

---

#### 4.2.3 GlyphData 类

**文件位置**：`src/ttf_parser/glyph_data.h`

**功能说明**：字形数据表，负责加载和解析字形数据

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `GlyphData(const TrueTypeFont&)` | 构造函数 | `ttf`: 字体对象引用 | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | 初始化加载 | `entry`: 表记录, `fin`: 文件流 | void |
| `Destroy()` | 释放资源 | - | void |
| `DumpInfo(XmlLogger&) const` | 输出信息到 XML | `logger`: XML 日志器 | void |
| `LoadGlyph(GlyphId)` | 加载指定字形 | `glyph_index`: 字形索引 | `const Glyph&` |

---

#### 4.2.4 CharacterToGlyphIndexMappingTable 类

**文件位置**：`src/ttf_parser/character_to_glyph_mapping_table.h`

**功能说明**：字符到字形索引的映射表，支持多种编码格式

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `CharacterToGlyphIndexMappingTable(const TrueTypeFont&)` | 构造函数 | `ttf`: 字体对象引用 | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | 初始化加载 | `entry`: 表记录, `fin`: 文件流 | void |
| `Destroy()` | 释放资源 | - | void |
| `DumpInfo(XmlLogger&) const` | 输出信息到 XML | `logger`: XML 日志器 | void |
| `GetGlyphIndex(UShort, UShort, UShort) const` | 获取字形索引 | `platform_id`, `encoding_id`, `ch`: 字符 | `GlyphId` |

**编码格式枚举**：

| 枚举值 | 名称 | 说明 |
|-------|------|------|
| 0 | `kByteEncodingTable` | 单字节编码表 |
| 2 | `kHighByteMappingThroughTable` | 高字节映射表 |
| 4 | `kSegmentMappingToDeltaValues` | 分段映射 |
| 6 | `kTrimmedTableMapping` | 修剪表映射 |
| 8 | `kMixed16BitAnd32BitCoverage` | 混合 16/32 位覆盖 |
| 10 | `kTrimmedArray` | 修剪数组 |
| 12 | `kSegmentedCoverage` | 分段覆盖 |
| 13 | `kManyToOneRangeMappings` | 多对一范围映射 |
| 14 | `kUnicodeVariationSequences` | Unicode 变体序列 |

---

#### 4.2.5 FontHeader 类

**文件位置**：`src/ttf_parser/font_header.h`

**功能说明**：字体头表，包含字体的全局度量信息

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `FontHeader(const TrueTypeFont&)` | 构造函数 | `ttf`: 字体对象引用 | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | 初始化加载 | `entry`: 表记录, `fin`: 文件流 | void |
| `DumpInfo(XmlLogger&) const` | 输出信息到 XML | `logger`: XML 日志器 | void |

**属性访问方法**：

| 方法名 | 返回类型 | 说明 |
|-------|---------|------|
| `x_min() const` | `FWord` | 字体边界最小 X |
| `y_min() const` | `FWord` | 字体边界最小 Y |
| `x_max() const` | `FWord` | 字体边界最大 X |
| `y_max() const` | `FWord` | 字体边界最大 Y |
| `loca_format() const` | `Short` | loca 表格式 (0=短偏移, 1=长偏移) |

---

#### 4.2.6 MaximumProfile 类

**文件位置**：`src/ttf_parser/maximum_profile.h`

**功能说明**：最大轮廓表，记录字体中各种最大值

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `MaximumProfile(const TrueTypeFont&)` | 构造函数 | `ttf`: 字体对象引用 | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | 初始化加载 | `entry`: 表记录, `fin`: 文件流 | void |
| `DumpInfo(XmlLogger&) const` | 输出信息到 XML | `logger`: XML 日志器 | void |

**属性访问方法**：

| 方法名 | 返回类型 | 说明 |
|-------|---------|------|
| `num_glyphs() const` | `UShort` | 字形数量 |
| `max_points() const` | `UShort` | 最大点数 |
| `max_contours() const` | `UShort` | 最大轮廓数 |
| `max_composite_points() const` | `UShort` | 复合字形最大点数 |
| `max_composite_contours() const` | `UShort` | 复合字形最大轮廓数 |
| `max_size_of_instructions() const` | `UShort` | 指令最大长度 |
| `max_component_depth() const` | `UShort` | 组件嵌套最大深度 |

---

#### 4.2.7 IndexToLocation 类

**文件位置**：`src/ttf_parser/index_to_location.h`

**功能说明**：索引定位表，提供字形在 glyf 表中的偏移位置

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `IndexToLocation(const TrueTypeFont&)` | 构造函数 | `ttf`: 字体对象引用 | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | 初始化加载 | `entry`: 表记录, `fin`: 文件流 | void |
| `Destroy()` | 释放资源 | - | void |
| `DumpInfo(XmlLogger&) const` | 输出信息到 XML | `logger`: XML 日志器 | void |
| `GetGlyphOffsetAndLength(GlyphId, ULong*, ULong*) const` | 获取字形偏移和长度 | `glyph_index`, `offset`, `length` | void |

---

#### 4.2.8 HorizontalHeader 类

**文件位置**：`src/ttf_parser/horizontal_header.h`

**功能说明**：水平布局头表，包含水平度量的元信息

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `HorizontalHeader(const TrueTypeFont&)` | 构造函数 | `ttf`: 字体对象引用 | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | 初始化加载 | `entry`: 表记录, `fin`: 文件流 | void |
| `DumpInfo(XmlLogger&) const` | 输出信息到 XML | `logger`: XML 日志器 | void |

**属性访问方法**：

| 方法名 | 返回类型 | 说明 |
|-------|---------|------|
| `num_hmetrics() const` | `UShort` | 水平度量数量 |

---

#### 4.2.9 HorizontalMetrics 类

**文件位置**：`src/ttf_parser/horizontal_metrics.h`

**功能说明**：水平度量表，包含每个字形的水平度量数据

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `HorizontalMetrics(const TrueTypeFont&)` | 构造函数 | `ttf`: 字体对象引用 | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | 初始化加载 | `entry`: 表记录, `fin`: 文件流 | void |
| `Destroy()` | 释放资源 | - | void |
| `DumpInfo(XmlLogger&) const` | 输出信息到 XML | `logger`: XML 日志器 | void |
| `GetLeftSideBearing(GlyphId) const` | 获取左侧间距 | `glyph_index`: 字形索引 | `FWord` |
| `GetAdvanceWidth(GlyphId) const` | 获取前进宽度 | `glyph_index`: 字形索引 | `UFword` |

---

#### 4.2.10 Naming_Table 类

**文件位置**：`src/ttf_parser/naming_table.h`

**功能说明**：命名表，包含字体的名称、版权等信息

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `Naming_Table(const TrueTypeFont&)` | 构造函数 | `ttf`: 字体对象引用 | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | 初始化加载 | `entry`: 表记录, `fin`: 文件流 | void |
| `Destroy()` | 释放资源 | - | void |
| `DumpInfo(XmlLogger&) const` | 输出信息到 XML | `logger`: XML 日志器 | void |

**平台 ID 枚举**：

| 枚举值 | 名称 | 说明 |
|-------|------|------|
| 0 | `kUnicode` | Unicode |
| 1 | `kMacintosh` | Macintosh |
| 2 | `kISO` | ISO |
| 3 | `kWindows` | Windows |
| 4 | `kCustom` | 自定义 |

---

#### 4.2.11 Os2AndWindowsMetrics 类

**文件位置**：`src/ttf_parser/os_2_and_windows_metrics.h`

**功能说明**：OS/2 和 Windows 度量表，包含字体的详细度量信息

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `Os2AndWindowsMetrics(const TrueTypeFont&)` | 构造函数 | `ttf`: 字体对象引用 | - |
| `Init(const TableRecordEntry*, std::ifstream&)` | 初始化加载 | `entry`: 表记录, `fin`: 文件流 | void |
| `DumpInfo(XmlLogger&) const` | 输出信息到 XML | `logger`: XML 日志器 | void |

**关键字段**：

| 字段名 | 类型 | 说明 |
|-------|------|------|
| `version_` | `UShort` | 版本号 |
| `us_weight_class_` | `UShort` | 字重等级 |
| `us_width_class_` | `UShort` | 宽度等级 |
| `panose_` | `PANOSE` | PANOSE 分类 |
| `ul_unicode_range_1_~4_` | `ULong` | Unicode 覆盖范围 |

---

### 4.3 工具模块

#### 4.3.1 XmlLogger 类

**文件位置**：`src/ttf_parser/xml_logger.h`

**功能说明**：XML 格式日志输出工具

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `XmlLogger(const QString&)` | 构造函数 | `out_path`: 输出路径 | - |
| `~XmlLogger()` | 析构函数 | - | - |
| `Error() const` | 检查是否有错误 | - | bool |
| `IncreaseIndent()` | 增加缩进 | - | void |
| `DecreaseIndent()` | 减少缩进 | - | void |
| `Print(const char*, ...)` | 打印格式化文本 | `format`: 格式字符串 | void |
| `Println(const char*, ...)` | 打印格式化文本并换行 | `format`: 格式字符串 | void |
| `WPrint(const wchar_t*, ...)` | 打印宽字符文本 | `format`: 格式字符串 | void |
| `WPrintln(const wchar_t*, ...)` | 打印宽字符文本并换行 | `format`: 格式字符串 | void |

---

#### 4.3.2 MemStream 类

**文件位置**：`src/ttf_parser/mem_stream.h`

**功能说明**：内存流读取工具，支持大端序数据读取

**公共方法**：

| 方法名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `MemStream(const void*, size_t)` | 构造函数 | `m`: 内存起始地址, `size`: 大小 | - |
| `~MemStream()` | 析构函数 | - | - |
| `Read(void*, size_t)` | 读取数据 | `buf`: 缓冲区, `size`: 大小 | void |
| `Seek(int)` | 移动游标 | `offset`: 偏移量 | void |
| `Tell()` | 获取当前位置 | - | int |
| `ReadBigEndian(void*, unsigned)` | 读取大端序数据 | `buf`: 缓冲区, `size`: 大小 | void |
| `ReadBigEndian(void*, unsigned, unsigned)` | 批量读取大端序数据 | `buf`, `unit_size`, `unit_num` | void |

**宏定义**：

| 宏名 | 功能说明 |
|-----|---------|
| `MREAD(s, b)` | 读取单个大端序数据 |
| `MREAD_N(s, b, n)` | 读取 n 个大端序数据 |

---

### 4.4 类型定义

**文件位置**：`src/ttf_parser/type.h`

**功能说明**：统一的数据类型定义

**基础类型映射**：

| 类型别名 | 底层类型 | 说明 |
|---------|---------|------|
| `Int8` | `signed char` | 8 位有符号整数 |
| `Int16` | `signed short` | 16 位有符号整数 |
| `Int32` | `signed int` | 32 位有符号整数 |
| `Int64` | `signed long long` | 64 位有符号整数 |
| `UInt8` | `unsigned char` | 8 位无符号整数 |
| `UInt16` | `unsigned short` | 16 位无符号整数 |
| `UInt32` | `unsigned int` | 32 位无符号整数 |
| `UInt64` | `unsigned long long` | 64 位无符号整数 |

**TTF 专用类型**：

| 类型别名 | 底层类型 | 说明 |
|---------|---------|------|
| `Byte` | `UInt8` | 字节 |
| `Char` | `Int8` | 字符 |
| `UShort` | `UInt16` | 无符号短整数 |
| `Short` | `Int16` | 有符号短整数 |
| `ULong` | `UInt32` | 无符号长整数 |
| `Long` | `Int32` | 有符号长整数 |
| `Fixed` | `UInt32` | 定点数 (16.16) |
| `FWord` | `Short` | 字体单位 |
| `UFword` | `UShort` | 无符号字体单位 |
| `F2Dot14` | `UInt16` | 2.14 定点数 |
| `LongDateTime` | `Int64` | 日期时间 |
| `GlyphId` | `UInt16` | 字形 ID |
| `Offset` | `UInt16` | 偏移量 |

**辅助函数**：

| 函数名 | 功能说明 | 参数 | 返回值 |
|-------|---------|------|-------|
| `F2Dot14ToFloat(F2Dot14)` | F2Dot14 转 float | `val`: 定点数值 | `float` |
| `TableRecordTagToULong(const char*)` | 标签转 ULong | `tag`: 4 字符标签 | `ULong` |

---

## 5. 编译与构建

### 5.1 依赖要求

| 依赖 | 版本要求 |
|-----|---------|
| Qt | 5.15+ 或 6.0+ |
| CMake | 3.16+ (可选) |

### 5.2 构建方式

#### 方式 1：Qt Creator

1. 打开 Qt Creator
2. 文件 → 打开文件或项目
3. 选择 `ttf_parser_qt.pro`
4. 选择构建套件
5. 点击运行

#### 方式 2：qmake 命令行

```bash
cd qt_ttf_parser
mkdir build
cd build
qmake ../ttf_parser_qt.pro
nmake  # 或 mingw32-make
```

#### 方式 3：CMake

```bash
cd qt_ttf_parser
mkdir build
cd build
cmake ..
cmake --build .
```

---

## 6. 项目目录结构

```
qt_ttf_parser/
├── CMakeLists.txt              # CMake 构建配置
├── ttf_parser_qt.pro           # Qt 构建配置
├── README.md                   # 项目说明
├── info.xml                    # 项目信息
├── .qtcreator/                 # Qt Creator 配置
│   └── ttf_parser_qt.pro.user
├── build_qmake/                # 构建输出目录
│   └── release/
└── src/                        # 源代码目录
    ├── main.cc                 # 程序入口
    ├── ui/                     # UI 组件
    │   ├── main_window.h/cc    # 主窗口
    │   └── glyph_widget.h/cc   # 字形渲染控件
    └── ttf_parser/             # TTF 解析核心
        ├── type.h              # 类型定义
        ├── ttf_table.h/cc      # 表基类
        ├── true_type_font.h/cc # 主解析类
        ├── glyph_data.h/cc     # 字形数据
        ├── font_header.h/cc    # 字体头
        ├── maximum_profile.h/cc # 最大轮廓
        ├── index_to_location.h/cc # 索引定位
        ├── character_to_glyph_mapping_table.h/cc # 字符映射
        ├── horizontal_header.h/cc # 水平头
        ├── horizontal_metrics.h/cc # 水平度量
        ├── naming_table.h/cc   # 命名表
        ├── os_2_and_windows_metrics.h/cc # OS/2 度量
        ├── post_script.h/cc    # PostScript
        ├── mem_stream.h/cc     # 内存流
        ├── xml_logger.h/cc     # XML 日志
        └── stdafx.h/cc         # 预编译头
```