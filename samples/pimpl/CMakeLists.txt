cmake_minimum_required(VERSION 2.8)
project(QiLangPimplSample)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

include_directories(".")

set(IDL_PIMPL pimpl/bar.idl.qi pimpl/foo.idl.qi)
set(IDL_TITI titi/titi.idl.qi)
#set(H "" pimpl/bar.hpp)
set(C src/barimpl.cpp)

include(../../qilang-tools-config.cmake)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

qi_gen_idl(pimpl_idlsrc CPP "pimpl" "${CMAKE_CURRENT_BINARY_DIR}"  ${IDL_PIMPL})
qi_gen_idl(titi_idlsrc CPP "titi" "${CMAKE_CURRENT_BINARY_DIR}"  ${IDL_TITI})

qi_create_lib(qipimpl ${C} ${pimpl_idlsrc} ${titi_idlsrc})
add_dependencies(qipimpl pimpl_idl_target titi_idl_target)

qi_use_lib(qipimpl qi)
qi_stage_lib(qipimpl)


qi_create_bin(pimp test.cpp)
qi_use_lib(pimp qipimpl)
