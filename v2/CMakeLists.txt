cmake_minimum_required(VERSION 2.8)
project(QiLang2)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

include_directories(".")

set(IDL_PIMPL pimpl/bar.idl.qi pimpl/foo.idl.qi)
set(IDL_TITI titi/titi.idl.qi)
#set(H "" pimpl/bar.hpp)
set(C src/barimpl.cpp)

include(../qilang-tools-config.cmake)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

qi_gen_idl(idlsrc CPP qilang2 ${CMAKE_CURRENT_BINARY_DIR} qilang2/node.idl.qi)
qi_create_lib(qilang2 ${idlsrc} src/nodeimpl.cpp)
qi_use_lib(qilang2 qi)
qi_stage_lib(qilang2)

add_dependencies(qilang2 qi_idl_qilang2)
#add_dependencies(qilang2 qic)

qi_create_bin(qilang2test src/main.cpp DEPENDS qilang2)
