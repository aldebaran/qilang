cmake_minimum_required(VERSION 2.8)
project(QiLang)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

include_directories(".")

set(H qilang/node.hpp
      qilang/parser.hpp
      qilang/formatter.hpp)

set(C src/node.cpp
      src/parser.cpp
      src/formatter_p.hpp
      src/formatter.cpp
      src/genobject.cpp)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

add_definitions("-DYYDEBUG")

qi_generate_src(
  ${CMAKE_CURRENT_BINARY_DIR}/lex.y.cpp
   SRC src/token.l src/grammar.y
   COMMENT "flexing ..."
   COMMAND ${FLEX_EXECUTABLE} -o lex.y.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/token.l
)

qi_generate_src(
  ${CMAKE_CURRENT_BINARY_DIR}/grammar.tab.cpp
   SRC src/grammar.y
   COMMENT "bisoning ..."
   COMMAND ${BISON_EXECUTABLE} -Wall -t -g=graph.txt -v -d -o grammar.tab.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/grammar.y
)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("src")

qi_create_lib(qilang
  ${H}
  ${C}
  ${CMAKE_CURRENT_BINARY_DIR}/lex.y.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/grammar.tab.cpp)

qi_use_lib(qilang qi)
qi_stage_lib(qilang)


qi_create_bin(qic src/qic_main.cpp)

qi_use_lib(qic qilang)
