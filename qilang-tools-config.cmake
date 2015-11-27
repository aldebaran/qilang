##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2014 Aldebaran Robotics
##

if (QICC_USER_EXECUTABLE)
  set(QICC_EXECUTABLE ${QICC_USER_EXECUTABLE})
else()
  find_package(qicc REQUIRED)
endif()

message(STATUS "Using qicc: ${QICC_EXECUTABLE}")

#! Generate qicc cpp files.
#
# \arg:OUT     an OUTPUT variable with a list of all generated files
# \arg:lang    the language to generate code to (only CPP is supported for the
#   moment)
# \arg:pkg     the package path (qi/foo/bar)
# \arg:dir     the directory where the files will be generated
# \flag:NOINTERFACE do not generate interface file
# \flag:NOLOCAL     do not generate local file
# \flag:NOREMOTE    do not generate remote file
# \group:FLAGS      flags to pass to qicc
#
# This function will set three variables:
# - ${OUT}_INTERFACE which contains the interfaces of your classes
# - ${OUT}_LOCAL which is the implementation of the interfaces as local
#   wrappers over your classes
# - ${OUT}_REMOTE which is the implementation of the interfaces as remote
#   proxies
function(qi_gen_idl OUT lang pkg dir)
  cmake_parse_arguments(ARG
    "NOINTERFACE;NOLOCAL;NOREMOTE"
    ""
    "FLAGS"
    ${ARGN})
  foreach(arg ${ARG_UNPARSED_ARGUMENTS})
    get_filename_component(absname "${arg}" ABSOLUTE)
    get_filename_component(absdir "${dir}" ABSOLUTE)
    get_filename_component(fout "${arg}" NAME_WE)
    file(MAKE_DIRECTORY ${absdir}/${pkg})

    if(NOT ARG_NOINTERFACE)
      qi_generate_src("${absdir}/${pkg}/${fout}.hpp"
        SRC "${absname}"
        COMMENT "Generating interface ${pkg}/${fout}.hpp"
        DEPENDS "${QICC_EXECUTABLE}" "${absname}"
        COMMAND "${QICC_EXECUTABLE}" -c cpp_interface "${absname}" -o "${absdir}/${pkg}/${fout}.hpp")
      list(APPEND _out "${absdir}/${pkg}/${fout}.hpp")
      set(${OUT}_INTERFACE "${absdir}/${pkg}/${fout}.hpp" PARENT_SCOPE)
    endif()

    if(NOT ARG_NOLOCAL)
      qi_generate_src("${absdir}/src/${fout}_p.hpp"
        SRC "${absname}"
        COMMENT "Generating local src/${fout}_p.hpp"
        DEPENDS "${QICC_EXECUTABLE}" "${absname}"
        COMMAND "${QICC_EXECUTABLE}" -c cpp_local "${absname}" -o "${absdir}/src/${fout}_p.hpp")
      list(APPEND _out "${absdir}/src/${fout}_p.hpp")
      set(${OUT}_LOCAL "${absdir}/src/${fout}_p.hpp" PARENT_SCOPE)
    endif()

    if(NOT ARG_NOREMOTE)
      qi_generate_src("${absdir}/src/${fout}remote.cpp"
        SRC "${absname}"
        COMMENT "Generating remote src/${fout}remote.cpp"
        DEPENDS "${QICC_EXECUTABLE}" "${absname}"
        COMMAND "${QICC_EXECUTABLE}" -c cpp_remote "${absname}" -o "${absdir}/src/${fout}remote.cpp")
      list(APPEND _out "${absdir}/src/${fout}remote.cpp")
      set(${OUT}_REMOTE "${absdir}/src/${fout}remote.cpp" PARENT_SCOPE)
    endif()
  endforeach()
  #this custom target ensure that all idl file are generated before building
  add_custom_target(qi_idl_${pkg} DEPENDS ${_out})
  set(${OUT} ${_out} PARENT_SCOPE)
endfunction()
