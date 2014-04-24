cmake_minimum_required(VERSION 2.8)
project(QiLangPimplSample)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

include_directories(".")

set(H pimpl/bar.hpp)
set(C src/bar.cpp src/barimpl.cpp)

qi_create_lib(qipimpl ${H} ${C})

qi_use_lib(qipimpl qi qitype)
qi_stage_lib(qipimpl)


qi_create_bin(pimp test.cpp)
qi_use_lib(pimp qipimpl)
