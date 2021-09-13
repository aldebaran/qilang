cmake_minimum_required(VERSION 2.8)
project(QiLang)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

enable_testing()

include_directories(".")

set(H qilang/api.hpp
      qilang/node.hpp
      qilang/visitor.hpp
      qilang/parser.hpp
      qilang/formatter.hpp
      qilang/packagemanager.hpp
      qilang/docparser.hpp
      qilang/pathformatter.hpp
   )

set(C src/codegen.cpp
      src/node.cpp
      src/parser.cpp
      src/parser_p.hpp
      src/formatter_p.hpp
      src/format_doc.cpp
      src/format_qilang.cpp
      src/format_sexpr.cpp
      src/format_cpp_interface.cpp
      src/format_cpp_local.cpp
      src/format_cpp_remote.cpp
      src/format_cpp_gmock.cpp
      src/format_anyvalue.cpp
      src/cpptype.hpp
      src/cpptype.cpp
      src/packagemanager.cpp
      src/qilang_signature.cpp
      src/qilang_metaobject.cpp
      src/docparser.cpp
      src/pathformatter.cpp)

find_package(FLEX NO_MODULE REQUIRED)
find_package(BISON NO_MODULE REQUIRED)

add_definitions("-DYYDEBUG")

qi_generate_src(
  ${CMAKE_CURRENT_BINARY_DIR}/lex.y.cpp
   SRC src/token.l src/grammar.y
   COMMENT "flexing ..."
   COMMAND ${FLEX_EXECUTABLE} --batch -o lex.y.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/token.l
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
  SHARED
  ${H}
  ${C}
  src/grammar.y
  src/token.l
  ${CMAKE_CURRENT_BINARY_DIR}/lex.y.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/grammar.tab.cpp)

qi_use_lib(qilang qi)
qi_stage_lib(qilang)


qi_create_bin(qicc src/qic_main.cpp DEPENDS qi qilang boost_chrono)
qi_stage_bin(qicc)

qi_stage_cmake(qilang-tools-config.cmake)

# include it go for testing!
include(${CMAKE_CURRENT_SOURCE_DIR}/qilang-tools-config.cmake)
if (QI_WITH_TESTS)
  add_subdirectory("tests")
endif()
