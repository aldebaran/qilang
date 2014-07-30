##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2014 Aldebaran Robotics
##

if (QIC_USER_EXECUTABLE)
  set(QIC_EXECUTABLE ${QIC_USER_EXECUTABLE})
else()
  find_package(qic REQUIRED)
endif()

message(STATUS "Using qic: ${QIC_EXECUTABLE}")
#!
# generate qic cpp files.
#
#
# \arg:OUT     an OUTPUT variable will a list of all generated files
# \arg:lang    the language to generate code to
# \arg:pkg     the package path (qi/foo/bar)
# \arg:dir     the directory where the files will be generated
function(qi_gen_idl OUT lang pkg dir)
  foreach(arg ${ARGN})
    get_filename_component(absname "${arg}" ABSOLUTE)
    get_filename_component(absdir "${dir}" ABSOLUTE)
    get_filename_component(fout "${arg}" NAME_WE)
    file(MAKE_DIRECTORY ${absdir}/${pkg})
    qi_generate_src("${absdir}/${pkg}/${fout}.hpp"
      SRC "${absname}"
      COMMENT "IDL: interface ${pkg}/${fout}.hpp ..."
      DEPENDS "${QIC_EXECUTABLE}" "${absname}"
      COMMAND "${QIC_EXECUTABLE}" -c cpp_interface "${absname}" -o "${absdir}/${pkg}/${fout}.hpp")

    qi_generate_src("${absdir}/src/${fout}.cpp"
      SRC "${absname}"
      COMMENT "IDL: bind src/${fout}.cpp ..."
      DEPENDS "${QIC_EXECUTABLE}" "${absname}"
      COMMAND "${QIC_EXECUTABLE}" -c cpp_bind "${absname}" -o "${absdir}/src/${fout}.cpp")

    qi_generate_src("${absdir}/src/${fout}_p.hpp"
      SRC "${absname}"
      COMMENT "IDL: local src/${fout}_p.hpp ..."
      DEPENDS "${QIC_EXECUTABLE}" "${absname}"
      COMMAND "${QIC_EXECUTABLE}" -c cpp_local "${absname}" -o "${absdir}/src/${fout}_p.hpp")

    qi_generate_src("${absdir}/src/${fout}remote.cpp"
      SRC "${absname}"
      COMMENT "IDL: remote src/${fout}remote.cpp ..."
      DEPENDS "${QIC_EXECUTABLE}" "${absname}"
      COMMAND "${QIC_EXECUTABLE}" -c cpp_remote "${absname}" -o "${absdir}/src/${fout}remote.cpp")

    list(APPEND _out "${absdir}/${pkg}/${fout}.hpp")
    list(APPEND _out "${absdir}/src/${fout}.cpp")
    list(APPEND _out "${absdir}/src/${fout}_p.hpp")
    list(APPEND _out "${absdir}/src/${fout}remote.cpp")
  endforeach()
  #this custom target ensure that all idl file are generated before building
  add_custom_target(qi_idl_${pkg} DEPENDS ${_out})
  set(${OUT} ${_out} PARENT_SCOPE)
endfunction()
