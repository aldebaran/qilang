cmake_minimum_required(VERSION 2.8)
project(QiLangPimplSample)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

include_directories(".")

set(H bar.hpp)
set(C bar.cpp barimpl.cpp)

qi_create_lib(qipimpl
  #${H}
  ${C})

qi_use_lib(qilang qi qitype)
qi_stage_lib(qilang)


qi_create_bin(qic src/qic_main.cpp)

qi_use_lib(qic qilang)
