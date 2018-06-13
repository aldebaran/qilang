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
# \flag:NOGMOCK     do not generate GMockups
# \flag:NO_INSTALL  do not generate install rule
# \group:FLAGS      flags to pass to qicc
#
# This function will set three variables:
# - ${OUT}_INTERFACE which contains the interfaces of your classes
# - ${OUT}_LOCAL which is the implementation of the interfaces as local
#   wrappers over your classes
# - ${OUT}_REMOTE which is the implementation of the interfaces as remote
#   proxies
# - ${OUT}_GMOCK which is mockups of the interfaces using GMock
function(qi_gen_idl OUT lang pkg dir)
  cmake_parse_arguments(ARG
    "NOINTERFACE;NOLOCAL;NOREMOTE;NOGMOCK;NO_INSTALL"
    ""
    "FLAGS"
    ${ARGN})

  foreach(rel_idl_path ${ARG_UNPARSED_ARGUMENTS})
    message(STATUS "Processing IDL file: ${rel_idl_path}")
    if (IS_ABSOLUTE "${rel_idl_path}")
        message(FATAL_ERROR "IDL path shall be relative, but was '${rel_idl_path}'")
    endif()
    # check the prefix is share/qi/idl/
    set(_expected_prefix "share/qi/idl/")
    string(LENGTH "${_expected_prefix}" _prefix_length)
    string(SUBSTRING "${rel_idl_path}" 0 "${_prefix_length}" _prefix)
    if (NOT _prefix STREQUAL _expected_prefix)
      if(NOT QI_GEN_IDL_SUPPORT_LEGACY_LAYOUT)
        message(FATAL_ERROR
                "IDL path '${rel_idl_path}' does not begin with '${_expected_prefix}'.")
      endif()
      set (_prefix "")
      set (_prefix_length 0)
      # strip the filename
      get_filename_component(package_subpackage "${rel_idl_path}" DIRECTORY)
    else()
      # strip the filename
      get_filename_component(prefix_package_subpackage "${rel_idl_path}" DIRECTORY)
      # strip the prefix
      string(SUBSTRING "${prefix_package_subpackage}" "${_prefix_length}" -1 package_subpackage)
    endif()
    get_filename_component(subpackage "${package_subpackage}" NAME)
    set(maybe_subpackage "${subpackage}/")
    if(subpackage STREQUAL package_subpackage)
      set(subpackage "")
      set(maybe_subpackage "")
    endif()

    get_filename_component(abs_gen_dest_dir "${dir}" ABSOLUTE)
    get_filename_component(abs_idl_path "${rel_idl_path}" ABSOLUTE)
    get_filename_component(dest_basename "${rel_idl_path}" NAME_WE)
    get_filename_component(dest_filename "${rel_idl_path}" NAME)
    file(MAKE_DIRECTORY ${abs_gen_dest_dir}/${package_subpackage})

    if(NOT ARG_NO_INSTALL)
      # each idl file shall be installed in the sdk only
      qi_install("${rel_idl_path}" COMPONENT devel DESTINATION "${_expected_prefix}${package_subpackage}")
    endif()

    if(NOT ARG_NOINTERFACE)
      set(generated_path "${abs_gen_dest_dir}/${package_subpackage}/${dest_basename}.hpp")
      qi_generate_src("${generated_path}"
        SRC "${abs_idl_path}"
        COMMENT "Generating interface ${generated_path}"
        DEPENDS "${QICC_EXECUTABLE}" "${abs_idl_path}"
        COMMAND "${QICC_EXECUTABLE}" -c cpp_interface "${abs_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
      list(APPEND _out "${generated_path}")
      list(APPEND _${OUT}_INTERFACE "${generated_path}")
      message(STATUS "Will generate C++ interface: ${generated_path}")
    endif()

    if(NOT ARG_NOLOCAL)
      set(generated_path "${abs_gen_dest_dir}/src/${maybe_subpackage}${dest_basename}_p.hpp")
      qi_generate_src(${generated_path}
        SRC "${abs_idl_path}"
        COMMENT "Generating local proxy wrapper ${generated_path}"
        DEPENDS "${QICC_EXECUTABLE}" "${abs_idl_path}"
        COMMAND "${QICC_EXECUTABLE}" -c cpp_local "${abs_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
      list(APPEND _out "${generated_path}")
      list(APPEND _${OUT}_LOCAL "${generated_path}")
      message(STATUS "Will generate C++ local proxy wrapper: ${generated_path}")
    endif()

    if(NOT ARG_NOREMOTE)
      set(generated_path "${abs_gen_dest_dir}/src/${maybe_subpackage}${dest_basename}remote.cpp")
      qi_generate_src("${generated_path}"
        SRC "${abs_idl_path}"
        COMMENT "Generating remote proxy implementation ${generated_path}"
        DEPENDS "${QICC_EXECUTABLE}" "${abs_idl_path}"
        COMMAND "${QICC_EXECUTABLE}" -c cpp_remote "${abs_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
      list(APPEND _out "${generated_path}")
      list(APPEND _${OUT}_REMOTE "${generated_path}")
      message(STATUS "Will generate C++ remote proxy implementation: ${generated_path}")
    endif()

    if(NOT ARG_NOGMOCK)
      set(generated_path "${abs_gen_dest_dir}/${package_subpackage}/gmock/${dest_basename}.hpp")
      qi_generate_src("${generated_path}"
        SRC "${abs_idl_path}"
        COMMENT "Generating C++ GMock ${generated_path}"
        DEPENDS "${QICC_EXECUTABLE}" "${abs_idl_path}"
        COMMAND "${QICC_EXECUTABLE}" -c cpp_gmock "${abs_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
      list(APPEND _out "${generated_path}")
      list(APPEND _${OUT}_GMOCK "${generated_path}")
      message(STATUS "Will generate C++ GMock: ${generated_path}")
    endif()
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
  if(NOT ARG_NOGMOCK)
    set(${OUT}_GMOCK ${_${OUT}_GMOCK} PARENT_SCOPE)
  endif()

  # This target is only useful for qilang unit tests.
  # It makes possible to add dependencies to qicc target.
  add_custom_target(qi_gen_idl_${pkg})
endfunction()

#! Generate C++ files and create a shared library out of them.
#
# \arg:package The package name, also the name of the resulting target
# \arg:dir The directory where the files will be generated
# \arg:depends The list of dependencies required by the generated library
# \flag:NO_INSTALL  do not generate install rule
#
# This function forwards the extra arguments to qi_gen_idl.
# \ref:qi_gen_idl
#
function(qi_gen_lib package destination)
  cmake_parse_arguments(ARG
    "NO_INSTALL"
    ""
    "DEPENDS;API_HEADER;IDL"
    ${ARGN})

  set(generated_files_var ${package}_generated)

  set(maybe_no_install "")
  if (ARG_NO_INSTALL)
      set(maybe_no_install "NO_INSTALL")
  endif()

  qi_gen_idl(
    ${generated_files_var}
    CPP
    ${package}
    ${destination}
    ${ARG_IDL}
    ${maybe_no_install}
    ${ARG_UNPARSED_ARGUMENTS}
  )

  if(NOT ARG_API_HEADER)
    set(ARG_API_HEADER "${package}/api.hpp")
  endif()

  include_directories("${destination}")

  qi_create_lib(
    ${package} SHARED
    ${maybe_no_install}
    ${ARG_API_HEADER}
    ${${generated_files_var}}
    ${ARG_IDL}

    DEPENDS
    qi
    ${ARG_DEPENDS}
  )

  get_filename_component(header_dir "${ARG_API_HEADER}" DIRECTORY)

  if (NOT ARG_NO_INSTALL)
    qi_install_header(
      ${${generated_files_var}_INTERFACE}
      ${${generated_files_var}_REMOTE}
      SUBFOLDER
      "${header_dir}"
    )
  endif()
endfunction()
