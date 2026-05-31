# TTF Parser - Qt 版本

这是原TTF Parser项目的Qt移植版本，提供了跨平台的图形界面。

## 项目结构

```
qt_ttf_parser/
├── CMakeLists.txt          # CMake构建配置
├── ttf_parser_qt.pro       # Qt构建配置
├── README.md               # 本文件
└── src/
    ├── main.cc             # 程序入口
    ├── ui/                 # UI组件
    │   ├── main_window.h/cc
    │   └── glyph_widget.h/cc
    └── ttf_parser/         # TTF解析核心库
```

## 编译方式

### 方式1：使用Qt Creator（推荐）
1. 打开Qt Creator
2. 文件 -> 打开文件或项目
3. 选择 `ttf_parser_qt.pro` 文件
4. 选择构建套件（Kit）
5. 点击运行按钮或按 Ctrl+R

### 方式2：使用qmake命令行
```bash
cd qt_ttf_parser
mkdir build
cd build
qmake ../ttf_parser_qt.pro
nmake  # 或 mingw32-make（MinGW）
```

### 方式3：使用CMake
```bash
cd qt_ttf_parser
mkdir build
cd build
cmake ..
cmake --build .
```

## 功能说明

1. **打开TTF文件** - 点击"Open TTF..."按钮选择TTF文件
2. **查看字形** - 通过滑块或输入框浏览所有字形
3. **字符映射** - 在"Character"输入框输入字符，点击"View"查看对应字形
4. **显示控制点** - 勾选"Show Points"显示字形的控制点（绿点是on-curve，蓝点是off-curve）
5. **导出XML** - 点击"Dump XML"导出TTF文件的详细信息

## 依赖

- Qt 5.15+ 或 Qt 6.0+
- CMake 3.16+（可选）

## 系统要求

- Windows 7+
- macOS 10.14+
- Linux (支持Qt的发行版)
