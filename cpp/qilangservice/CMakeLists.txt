cmake_minimum_required(VERSION 2.8)
project(@ProjectName@)

find_package(qibuild)
find_package(qimodule)
find_package(qilang-tools)

include_directories(".")

qi_gen_idl(@projectname@_idl CPP "@projectname@" "${CMAKE_CURRENT_BINARY_DIR}" @projectname@/@projectname@.idl.qi NOLOCAL)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PUBLIC_HEADERS
  ${@projectname@_idl_INTERFACE}
  @projectname@/api.hpp
)
qi_install_header(
  ${PUBLIC_HEADERS}
  SUBFOLDER @projectname@
)

set(SRCS
  ${@projectname@_idl_BIND}
  ${@projectname@_idl_REMOTE}
)

qi_create_lib(@projectname@lib
  SRC ${PUBLIC_HEADERS} ${SRCS}
  DEPENDS QI
)
qi_stage_lib(@projectname@lib)

qi_create_module(@projectname@
  SRC
    src/@projectname@impl.cpp
  DEPENDS
    QI
    @PROJECTNAME@LIB
)
