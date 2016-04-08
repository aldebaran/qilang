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

message(STATUS "Using qicc: ${QICC_EXECUTABLE} SEE ME")

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

  foreach(rel_idl_path ${ARG_UNPARSED_ARGUMENTS})
    message(STATUS "Processing IDL file: ${rel_idl_path}")
    get_filename_component(package_and_subpackage "${rel_idl_path}" DIRECTORY)
    get_filename_component(subpackage "${package_and_subpackage}" NAME)
    set(maybe_subpackage "${subpackage}/")
    if("${subpackage}" STREQUAL "${package_and_subpackage}")
      set(subpackage "")
      set(maybe_subpackage "")
    endif()

    get_filename_component(abs_idl_path "${rel_idl_path}" ABSOLUTE)
    get_filename_component(abs_dest_dir "${dir}" ABSOLUTE)
    get_filename_component(dest_filename "${rel_idl_path}" NAME_WE)
    file(MAKE_DIRECTORY ${abs_dest_dir}/${package_and_subpackage})

    if(NOT ARG_NOINTERFACE)
      set(generated_path "${abs_dest_dir}/${package_and_subpackage}/${dest_filename}.hpp")
      qi_generate_src("${generated_path}"
        SRC "${abs_idl_path}"
        COMMENT "Generating interface ${generated_path}"
        DEPENDS "${QICC_EXECUTABLE}" "${abs_idl_path}"
        COMMAND "${QICC_EXECUTABLE}" -c cpp_interface "${abs_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
      list(APPEND _out "${generated_path}")
      list(APPEND _${OUT}_INTERFACE "${generated_path}")
      message(STATUS "Will generate C++ interface header: ${generated_path}")
    endif()

    if(NOT ARG_NOLOCAL)
      set(generated_path "${abs_dest_dir}/src/${maybe_subpackage}${dest_filename}_p.hpp")
      qi_generate_src(${generated_path}
        SRC "${abs_idl_path}"
        COMMENT "Generating local ${generated_path}"
        DEPENDS "${QICC_EXECUTABLE}" "${abs_idl_path}"
        COMMAND "${QICC_EXECUTABLE}" -c cpp_local "${abs_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
      list(APPEND _out "${generated_path}")
      list(APPEND _${OUT}_LOCAL "${generated_path}")
      message(STATUS "Will generate C++ private header: ${generated_path}")
    endif()

    if(NOT ARG_NOREMOTE)
      set(generated_path "${abs_dest_dir}/src/${subpackage}/${dest_filename}remote.cpp")
      qi_generate_src("${generated_path}"
        SRC "${abs_idl_path}"
        COMMENT "Generating remote ${generated_path}"
        DEPENDS "${QICC_EXECUTABLE}" "${abs_idl_path}"
        COMMAND "${QICC_EXECUTABLE}" -c cpp_remote "${abs_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
      list(APPEND _out "${generated_path}")
      list(APPEND _${OUT}_REMOTE "${generated_path}")
      message(STATUS "Will generate C++ proxy implementation: ${generated_path}")
    endif()

    # each idl file shall be copied in the sdk folder
    get_filename_component(name "${rel_idl_path}" NAME)
    set(staged_idl_path "${QI_SDK_DIR}/${QI_SDK_SHARE}/qi/idl/${package_and_subpackage}")
    make_directory("${staged_idl_path}")
    add_custom_target(
      copy-idl-file-${dest_filename} ALL
      COMMAND ${CMAKE_COMMAND} -E copy "${abs_idl_path}" "${staged_idl_path}"
    )
    # each idl file shall be installed in the sdk
    qi_install_data("${rel_idl_path}" SUBFOLDER "qi/idl")
  endforeach()

  # Bounce out variables
  set(${OUT} ${_out} PARENT_SCOPE)
  if(NOT ARG_NOINTERFACE)
    set(${OUT}_INTERFACE ${_${OUT}_INTERFACE} PARENT_SCOPE)
  endif()
  if(NOT ARG_NOLOCAL)
    set(${OUT}_LOCAL ${_${OUT}_LOCAL} PARENT_SCOPE)
  endif()
  if(NOT ARG_NOREMOTE)
    set(${OUT}_REMOTE ${_${OUT}_REMOTE} PARENT_SCOPE)
  endif()

  # This target is only useful for qilang unit tests.
  # It makes possible to add dependencies to qicc target.
  add_custom_target(qi_gen_idl_${pkg})
endfunction()
