QT       += core widgets

TARGET = ttf_parser_qt
TEMPLATE = app

CONFIG += c++17

SOURCES += \
    src/main.cc \
    src/ui/main_window.cc \
    src/ui/glyph_widget.cc \
    src/ttf_parser/ttf_table.cc \
    src/ttf_parser/true_type_font.cc \
    src/ttf_parser/glyph_data.cc \
    src/ttf_parser/maximum_profile.cc \
    src/ttf_parser/character_to_glyph_mapping_table.cc \
    src/ttf_parser/horizontal_header.cc \
    src/ttf_parser/horizontal_metrics.cc \
    src/ttf_parser/index_to_location.cc \
    src/ttf_parser/font_header.cc \
    src/ttf_parser/os_2_and_windows_metrics.cc \
    src/ttf_parser/naming_table.cc \
    src/ttf_parser/xml_logger.cc \
    src/ttf_parser/mem_stream.cc \
    src/ttf_parser/stdafx.cc \
    src/ttf_parser/post_script.cc

HEADERS += \
    src/ui/main_window.h \
    src/ui/glyph_widget.h \
    src/ttf_parser/ttf_table.h \
    src/ttf_parser/true_type_font.h \
    src/ttf_parser/glyph_data.h \
    src/ttf_parser/maximum_profile.h \
    src/ttf_parser/character_to_glyph_mapping_table.h \
    src/ttf_parser/horizontal_header.h \
    src/ttf_parser/horizontal_metrics.h \
    src/ttf_parser/index_to_location.h \
    src/ttf_parser/font_header.h \
    src/ttf_parser/os_2_and_windows_metrics.h \
    src/ttf_parser/naming_table.h \
    src/ttf_parser/xml_logger.h \
    src/ttf_parser/mem_stream.h \
    src/ttf_parser/stdafx.h \
    src/ttf_parser/type.h \
    src/ttf_parser/post_script.h

INCLUDEPATH += \
    src

win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
}
