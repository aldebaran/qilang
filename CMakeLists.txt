cmake_minimum_required(VERSION 2.8)
project(QiC)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

include_directories(".")

set(H qilang/node.hpp
      qilang/parser.hpp)

set(C src/node.cpp
      src/parser.cpp)

find_package(FLEX)
find_package(BISON)

if(FLEX_EXECUTABLE AND BISON_EXECUTABLE)
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
     COMMAND ${BISON_EXECUTABLE} -d -o grammar.tab.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/grammar.y
  )
else()
  message("flex or bison not found, using pregenerated files")
  qi_generate_src(
    ${CMAKE_CURRENT_BINARY_DIR}/lex.y.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/grammar.tab.cpp
    COMMENT "copying generated files"
    SRC
        src/pregenerated/lex.y.cpp
        src/pregenerated/grammar.tab.cpp
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/pregenerated
        ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()

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
