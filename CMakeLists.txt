cmake_minimum_required(VERSION 3.16)
project(ttf_parser_qt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

set(SOURCES
  src/main.cc
  src/ui/main_window.cc
  src/ui/glyph_widget.cc
  src/ttf_parser/ttf_table.cc
  src/ttf_parser/true_type_font.cc
  src/ttf_parser/glyph_data.cc
  src/ttf_parser/maximum_profile.cc
  src/ttf_parser/character_to_glyph_mapping_table.cc
  src/ttf_parser/horizontal_header.cc
  src/ttf_parser/horizontal_metrics.cc
  src/ttf_parser/index_to_location.cc
  src/ttf_parser/font_header.cc
  src/ttf_parser/os_2_and_windows_metrics.cc
  src/ttf_parser/naming_table.cc
  src/ttf_parser/xml_logger.cc
  src/ttf_parser/mem_stream.cc
  src/ttf_parser/stdafx.cc
  src/ttf_parser/post_script.cc
)

set(HEADERS
  src/ui/main_window.h
  src/ui/glyph_widget.h
  src/ttf_parser/ttf_table.h
  src/ttf_parser/true_type_font.h
  src/ttf_parser/glyph_data.h
  src/ttf_parser/maximum_profile.h
  src/ttf_parser/character_to_glyph_mapping_table.h
  src/ttf_parser/horizontal_header.h
  src/ttf_parser/horizontal_metrics.h
  src/ttf_parser/index_to_location.h
  src/ttf_parser/font_header.h
  src/ttf_parser/os_2_and_windows_metrics.h
  src/ttf_parser/naming_table.h
  src/ttf_parser/xml_logger.h
  src/ttf_parser/mem_stream.h
  src/ttf_parser/stdafx.h
  src/ttf_parser/type.h
  src/ttf_parser/post_script.h
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME}
  ${SOURCES}
  ${HEADERS}
)

target_link_libraries(${PROJECT_NAME}
  Qt6::Core
  Qt6::Widgets
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
