cmake_minimum_required(VERSION 2.8)
project(Test)

find_package(qibuild)
find_package(qimodule)
find_package(qilang-tools)

qi_gen_idl(test_idl CPP "test" "${CMAKE_CURRENT_BINARY_DIR}" test/test.idl.qi)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(".")

set(PUBLIC_HEADERS
  ${test_idl_INTERFACE}
  test/api.hpp
)
qi_install_header(
  ${PUBLIC_HEADERS}
  SUBFOLDER test
)

set(SRCS
  ${test_idl_LOCAL}
  ${test_idl_REMOTE}
)

qi_create_lib(testlib SHARED
  SRC ${PUBLIC_HEADERS} ${SRCS}
  DEPENDS QI
)
qi_stage_lib(testlib)

qi_create_module(test
  SRC
    src/testimpl.cpp
  DEPENDS
    QI
    TESTLIB
)

qi_create_bin(server server.cpp DEPENDS QI TESTLIB)
qi_create_bin(client client.cpp DEPENDS QI TESTLIB)
