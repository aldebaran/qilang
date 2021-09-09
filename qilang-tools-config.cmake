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
    "NOINTERFACE;NOLOCAL;NOREMOTE;NOGMOCK"
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
    get_filename_component(abs_gen_dest_dir "${dir}" ABSOLUTE)
    get_filename_component(dest_basename "${rel_idl_path}" NAME_WE)
    get_filename_component(dest_filename "${rel_idl_path}" NAME)
    file(MAKE_DIRECTORY ${abs_gen_dest_dir}/${package_and_subpackage})

    # each idl file shall be copied in the sdk folder
    get_filename_component(name "${rel_idl_path}" NAME)
    set(staged_idl_dir "${QI_SDK_DIR}/${QI_SDK_SHARE}/qi/idl/${package_and_subpackage}")
    make_directory("${staged_idl_dir}")
    message(STATUS "Will mirror IDL file: ${abs_idl_path} to ${staged_idl_dir}")
    set(mirror_idl_file_target mirror-idl-file-${dest_filename})
    set(staged_idl_path "${staged_idl_dir}/${dest_filename}")
    if(UNIX)
      add_custom_target(
          ${mirror_idl_file_target} ALL
          COMMAND ${CMAKE_COMMAND} -E create_symlink "${abs_idl_path}" "${staged_idl_path}"
       )
    else(UNIX)
      add_custom_target(
        ${mirror_idl_file_target} ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${abs_idl_path}" "${staged_idl_path}"
      )
    endif(UNIX)

    # each idl file shall be installed in the sdk only
    qi_install("${rel_idl_path}" COMPONENT devel DESTINATION "${QI_SDK_SHARE}/qi/idl")

    if(NOT ARG_NOINTERFACE)
      set(generated_path "${abs_gen_dest_dir}/${package_and_subpackage}/${dest_basename}.hpp")
      qi_generate_src("${generated_path}"
        SRC "${abs_idl_path}"
        COMMENT "Generating interface ${generated_path}"
        DEPENDS "${QICC_EXECUTABLE}" "${staged_idl_path}" ${mirror_idl_file_target}
        COMMAND "${QICC_EXECUTABLE}" -c cpp_interface "${staged_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
      list(APPEND _out "${generated_path}")
      list(APPEND _${OUT}_INTERFACE "${generated_path}")
      message(STATUS "Will generate C++ interface: ${generated_path}")
    endif()

    if(NOT ARG_NOLOCAL)
      set(generated_path "${abs_gen_dest_dir}/src/${maybe_subpackage}${dest_basename}_p.hpp")
      qi_generate_src(${generated_path}
        SRC "${abs_idl_path}"
        COMMENT "Generating local proxy wrapper ${generated_path}"
        DEPENDS "${QICC_EXECUTABLE}" "${staged_idl_path}" ${mirror_idl_file_target}
        COMMAND "${QICC_EXECUTABLE}" -c cpp_local "${staged_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
      list(APPEND _out "${generated_path}")
      list(APPEND _${OUT}_LOCAL "${generated_path}")
      message(STATUS "Will generate C++ local proxy wrapper: ${generated_path}")
    endif()

    if(NOT ARG_NOREMOTE)
      set(generated_path "${abs_gen_dest_dir}/src/${subpackage}/${dest_basename}remote.cpp")
      qi_generate_src("${generated_path}"
        SRC "${abs_idl_path}"
        COMMENT "Generating remote proxy implementation ${generated_path}"
        DEPENDS "${QICC_EXECUTABLE}" "${staged_idl_path}" ${mirror_idl_file_target}
	COMMAND readelf -d "${CMAKE_CURRENT_SOURCE_DIR}/../build-qisdk/sdk/bin/qicc" | head -20 && readelf -d "/home/runner/.local/share/qi/toolchains/qisdk/libqi/lib/libqi.so" | head -20 && "${QICC_EXECUTABLE}" -c cpp_remote "${staged_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
      list(APPEND _out "${generated_path}")
      list(APPEND _${OUT}_REMOTE "${generated_path}")
      message(STATUS "Will generate C++ remote proxy implementation: ${generated_path}")
    endif()

    if(NOT ARG_NOGMOCK)
        set(generated_path "${abs_gen_dest_dir}/${package_and_subpackage}/gmock/${dest_basename}.hpp")
        qi_generate_src("${generated_path}"
          SRC "${abs_idl_path}"
          COMMENT "Generating C++ GMock ${generated_path}"
          DEPENDS "${QICC_EXECUTABLE}" "${staged_idl_path}" ${mirror_idl_file_target}
          COMMAND "${QICC_EXECUTABLE}" -c cpp_gmock "${staged_idl_path}" -o "${generated_path}" -t ${QI_SDK_DIR})
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
#
# This function forwards the extra arguments to qi_gen_idl.
# \ref:qi_gen_idl
#
function(qi_gen_lib package destination)
  cmake_parse_arguments(ARG
    ""
    ""
    "DEPENDS;API_HEADER;IDL"
    ${ARGN})

  set(generated_file_list ${package}_generated)

  qi_gen_idl(
    ${generated_file_list}
    CPP
    ${package}
    ${destination}
    ${ARG_IDL}
    ${ARG_UNPARSED_ARGUMENTS}
  )

  if(NOT ARG_API_HEADER)
    set(ARG_API_HEADER "${package}/api.hpp")
  endif()

  include_directories("${destination}")

  qi_create_lib(
    ${package} SHARED

    ${ARG_API_HEADER}
    ${${generated_file_list}}
    ${ARG_IDL}

    DEPENDS
    qi
    ${ARG_DEPENDS}
  )

  get_filename_component(header_dir "${ARG_API_HEADER}" DIRECTORY)

  qi_install_header(
    ${${package}_INTERFACE}
    ${${package}_REMOTE}
    SUBFOLDER
    "${header_dir}"
  )

endfunction()
