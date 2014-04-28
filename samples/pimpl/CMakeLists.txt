cmake_minimum_required(VERSION 2.8)
project(QiLangPimplSample)
find_package(qibuild)
qi_sanitize_compile_flags(HIDDEN_SYMBOLS)

include_directories(".")

set(IDL_PIMPL pimpl/bar.idl.qi pimpl/foo.idl.qi)
set(IDL_TITI titi/titi.idl.qi)
#set(H "" pimpl/bar.hpp)
set(C src/barimpl.cpp)

find_package(qic REQUIRED)
function(qi_gen_idl OUT LANG PKG DIR)
  foreach(arg ${ARGN})
    get_filename_component(absname "${arg}" ABSOLUTE)
    get_filename_component(absdir "${DIR}" ABSOLUTE)
    message(STATUS "generating '${QIC_EXECUTABLE}' hpp IDL for: ${arg}")
    get_filename_component(fout "${arg}" NAME_WE)
    qi_generate_src("${DIR}/${PKG}/${fout}.hpp"
      SRC "${absname}"
      COMMENT "IDL: generation headers ..."
      DEPENDS "${QIC_EXECUTABLE}"
      COMMAND "${QIC_EXECUTABLE}" -c cppi "${absname}" -o "${absdir}/${PKG}/${fout}.hpp")

    qi_generate_src("${absdir}/src/${fout}.cpp"
      SRC "${absname}"
      COMMENT "IDL: generation cpp ..."
      DEPENDS "${QIC_EXECUTABLE}"
      COMMAND "${QIC_EXECUTABLE}" -c cppr "${absname}" -o "${absdir}/src/${fout}.cpp")
    list(APPEND _out "${absdir}/${PKG}/${fout}.hpp")
    list(APPEND _out "${absdir}/src/${fout}.cpp")
  endforeach()
  add_custom_target(${PKG}_idl_target DEPENDS ${_out})
  set(${OUT} ${_out} PARENT_SCOPE)
endfunction()

qi_gen_idl(pimpl_idlsrc CPP "pimpl" "${CMAKE_CURRENT_SOURCE_DIR}"  ${IDL_PIMPL})
qi_gen_idl(titi_idlsrc CPP "titi" "${CMAKE_CURRENT_SOURCE_DIR}"  ${IDL_TITI})

qi_create_lib(qipimpl ${C} ${pimpl_idlsrc} ${titi_idlsrc})
add_dependencies(qipimpl pimpl_idl_target titi_idl_target)

qi_use_lib(qipimpl qi qitype)
qi_stage_lib(qipimpl)


qi_create_bin(pimp test.cpp)
qi_use_lib(pimp qipimpl)
