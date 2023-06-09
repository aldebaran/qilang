if(NOT TARGET qilang::qicc)
  include(CMakeFindDependencyMacro)
  find_dependency(qilang)
endif()

#[=======================================================================[.rst:
Generate source files from an IDL interface file.

.. code-block:: cmake

  qi_gen_idl(
    <output> <lang> <pkg> <dir> <idl_files>...
    [NOINTERFACE] [NOLOCAL] [NOREMOTE] [NOGMOCK]
    [NO_INSTALL]
    [FLAGS <flags>...]
  )

The function takes the following arguments:
- ``<output>``: a prefix that is used to declare output variables.
- ``<lang>``: the language to generate code to. Only "CPP" is supported at the moment.
- ``<pkg>``: the package name.
- ``<dir>``: the directory where the files will be generated.
- ``<idl_files>``: the list of IDL file paths. Each path must be relative and
  start with "share/qi/idl/".
- ``FLAGS``: list of flags to pass to ``qicc``.
- ``NOINTERFACE``: do not generate interface file.
- ``NOLOCAL``: do not generate local file.
- ``NOREMOTE``: do not generate remote file.
- ``NOGMOCK``: do not generate GMockups.
- ``NO_INSTALL``: do not generate install rule.

The function sets the following variables:
- ``<output>``: the list of all generated files.
- ``<output>_INTERFACE``: the list of interfaces of your classes.
- ``<output>_LOCAL``: the list of source files of implementation of the
  interfaces as local wrappers over your classes.
- ``<output>_REMOTE``: the list of source files of implementation of the
  interfaces as remote proxies.
- ``<output>_GMOCK`` which is the list of source files of mockups the
  interfaces using GoogleMock.
#]=======================================================================]
function(qi_gen_idl output lang pkg dir)
  cmake_parse_arguments(
    ARG
    "NOINTERFACE;NOLOCAL;NOREMOTE;NOGMOCK;NO_INSTALL"
    ""
    "FLAGS"
    ${ARGN}
  )

  foreach(rel_idl_path ${ARG_UNPARSED_ARGUMENTS})
    message(STATUS "Processing IDL file: ${rel_idl_path}")
    if(IS_ABSOLUTE "${rel_idl_path}")
      message(FATAL_ERROR "IDL path shall be relative, but was '${rel_idl_path}'")
    endif()
    # Check that the path of the IDL starts with "share/qi/idl/"
    set(_expected_prefix share/qi/idl/)
    string(LENGTH "${_expected_prefix}" _prefix_length)
    string(SUBSTRING "${rel_idl_path}" 0 "${_prefix_length}" _prefix)
    if (NOT _prefix STREQUAL _expected_prefix)
      message(FATAL_ERROR
              "IDL path '${rel_idl_path}' does not begin with '${_expected_prefix}'.")
      return()
    else()
      # Strip the filename
      get_filename_component(prefix_package_subpackage "${rel_idl_path}" DIRECTORY)
      # Strip the prefix
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
    file(MAKE_DIRECTORY "${abs_gen_dest_dir}/${package_subpackage}")

    if(NOT ARG_NO_INSTALL)
      # Each idl file shall be installed in the sdk only.
      install(
        FILES "${rel_idl_path}" 
        DESTINATION "${_expected_prefix}${package_subpackage}"
        COMPONENT devel
      )
    endif()

    set(sdk_dir "${CMAKE_BINARY_DIR}/sdk")

    if(NOT ARG_NOINTERFACE)
      set(generated_dir "${abs_gen_dest_dir}/${package_subpackage}")
      file(MAKE_DIRECTORY "${generated_dir}")
      set(generated_path "${generated_dir}/${dest_basename}.hpp")
      add_custom_command(
        OUTPUT "${generated_path}"
        COMMAND
          qilang::qicc
            -c cpp_interface
            "${abs_idl_path}"
            -o "${generated_path}"
            -t "${sdk_dir}"
        DEPENDS
          qilang::qicc
          "${abs_idl_path}"
        COMMENT "Generating interface ${generated_path}"
      )
      list(APPEND _out "${generated_path}")
      list(APPEND _${output}_INTERFACE "${generated_path}")
      message(STATUS "Will generate C++ interface: ${generated_path}")
    endif()

    if(NOT ARG_NOLOCAL)
      set(generated_dir "${abs_gen_dest_dir}/src/${maybe_subpackage}")
      file(MAKE_DIRECTORY "${generated_dir}")
      set(generated_path "${generated_dir}/${dest_basename}_p.hpp")
      add_custom_command(
        OUTPUT "${generated_path}"
        COMMAND
          qilang::qicc
            -c cpp_local
            "${abs_idl_path}"
            -o "${generated_path}"
            -t ${sdk_dir}
        DEPENDS
          qilang::qicc
          "${abs_idl_path}"
        COMMENT "Generating local proxy wrapper ${generated_path}"
      )
      list(APPEND _out "${generated_path}")
      list(APPEND _${output}_LOCAL "${generated_path}")
      message(STATUS "Will generate C++ local proxy wrapper: ${generated_path}")
    endif()

    if(NOT ARG_NOREMOTE)
      set(generated_dir "${abs_gen_dest_dir}/src/${maybe_subpackage}")
      file(MAKE_DIRECTORY "${generated_dir}")
      set(generated_path "${generated_dir}/${dest_basename}remote.cpp")
      add_custom_command(
        OUTPUT "${generated_path}"
        COMMAND
          qilang::qicc
            -c cpp_remote
            "${abs_idl_path}"
            -o "${generated_path}"
            -t ${sdk_dir}
        DEPENDS
          qilang::qicc
          "${abs_idl_path}"
        COMMENT "Generating remote proxy implementation ${generated_path}"
      )
      list(APPEND _out "${generated_path}")
      list(APPEND _${output}_REMOTE "${generated_path}")
      message(STATUS "Will generate C++ remote proxy implementation: ${generated_path}")
    endif()

    if(NOT ARG_NOGMOCK)
      set(generated_dir "${abs_gen_dest_dir}/${package_subpackage}/gmock/")
      file(MAKE_DIRECTORY "${generated_dir}")
      set(generated_path "${generated_dir}/${dest_basename}.hpp")
      add_custom_command(
        OUTPUT "${generated_path}"
        COMMAND
          qilang::qicc
            -c cpp_gmock
            "${abs_idl_path}"
            -o "${generated_path}"
            -t ${sdk_dir}
        DEPENDS
          qilang::qicc
          "${abs_idl_path}"
        COMMENT "Generating C++ GMock ${generated_path}"
      )
      list(APPEND _out "${generated_path}")
      list(APPEND _${output}_GMOCK "${generated_path}")
      message(STATUS "Will generate C++ GMock: ${generated_path}")
    endif()
  endforeach()

  # Bounce output variables
  set(${output} ${_out} PARENT_SCOPE)
  if(NOT ARG_NOINTERFACE)
    set(${output}_INTERFACE ${_${output}_INTERFACE} PARENT_SCOPE)
  endif()
  if(NOT ARG_NOLOCAL)
    set(${output}_LOCAL ${_${output}_LOCAL} PARENT_SCOPE)
  endif()
  if(NOT ARG_NOREMOTE)
    set(${output}_REMOTE ${_${output}_REMOTE} PARENT_SCOPE)
  endif()
  if(NOT ARG_NOGMOCK)
    set(${output}_GMOCK ${_${output}_GMOCK} PARENT_SCOPE)
  endif()
endfunction()

#[=======================================================================[.rst:
Generate source files and create a shared library out of them.

.. code-block:: cmake

  qi_gen_lib(
    <pkg> <dir> [IDL] <idl_files>...
    [NO_INSTALL]
    [API_HEADER <api_header>]
    [DEPENDS <deps>...]
  )

- ``<pkg>``: The package name, also the name of the resulting target.
- ``<dir>``: the directory where the files will be generated.
- ``<idl_files>``: the list of IDL file paths. Each path must be relative and
  start with "share/qi/idl/".
- ``DEPENDS``: the list of dependencies required by the generated library.
- ``API_HEADER``: the path to a "api" header, if any. If none is given, it
  will default to ``<pkg>/api.hpp``.
- ``NO_INSTALL``: do not generate install rule.

If an installation rule is generated, then the library target is exported
in the ``<pkg>-targets`` export set name.

This function forwards the extra arguments to ``qi_gen_idl``.
#]=======================================================================]
function(qi_gen_lib pkg dir)
  cmake_parse_arguments(
    ARG
    "NO_INSTALL"
    "API_HEADER"
    "DEPENDS;IDL"
    ${ARGN}
  )

  set(idl_files ${ARG_IDL} ${ARG_UNPARSED_ARGUMENTS})

  set(maybe_no_install "")
  if (ARG_NO_INSTALL)
      set(maybe_no_install "NO_INSTALL")
  endif()

  qi_gen_idl(
    generated
    CPP
    ${pkg}
    ${dir}
    ${idl_files}
    ${maybe_no_install}
  )

  if(NOT ARG_API_HEADER)
    set(ARG_API_HEADER "${pkg}/api.hpp")
  endif()

  if(NOT TARGET qi::qi)
    find_package(qi QUIET)
  endif()

  add_library("${pkg}" SHARED)

  target_sources(
    "${pkg}"
    PUBLIC
      FILE_SET idl_interface_headers
        TYPE HEADERS
        FILES
          ${generated_INTERFACE}
        BASE_DIRS "${dir}"
      FILE_SET idl_gmock_headers
        TYPE HEADERS
        FILES
          ${generated_GMOCK}
        BASE_DIRS "${dir}"
      FILE_SET api_header
        TYPE HEADERS
        FILES
          ${ARG_API_HEADER}
    PRIVATE
      ${generated_LOCAL}
      ${generated_REMOTE}
      ${idl_files}
  )

  target_include_directories(
    "${pkg}"
    PUBLIC
      $<BUILD_INTERFACE:${dir}>
  )

  target_link_libraries(
    "${pkg}"
    PUBLIC
      qi::qi
      ${ARG_DEPENDS}
  )

  if (NOT ARG_NO_INSTALL)
    install(
      TARGETS "${pkg}"
      EXPORT "${pkg}-targets"
      RUNTIME COMPONENT runtime
      LIBRARY COMPONENT runtime
      FILE_SET idl_interface_headers devel
      FILE_SET idl_gmock_headers devel
      FILE_SET api_header devel
    )
  endif()
endfunction()
