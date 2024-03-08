cmake_minimum_required(VERSION 3.23)
project(Test)

find_package(qimodule)
find_package(qilang-tools)
find_package(qi)

qi_gen_idl(test_idl CPP "test" "${CMAKE_CURRENT_BINARY_DIR}" share/qi/idl/test/test.idl.qi)

add_library(testlib SHARED)

target_sources(
    testlib
    PUBLIC
    FILE_SET HEADERS
      BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
      FILES
        ${test_idl_INTERFACE}
        test/api.hpp
    PRIVATE
      ${test_idl_LOCAL}
      ${test_idl_REMOTE}
)

target_include_directories(
  testlib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(testlib PUBLIC qi::qi)

install(
    TARGETS testlib
    RUNTIME
    FILE_SET HEADERS
)

qi_add_module(test src/testimpl.cpp)
target_link_libraries(test PRIVATE testlib)

add_executable(server)
target_sources(server PRIVATE server.cpp)
target_link_libraries(server PRIVATE qi::qi testlib)

add_executable(client)
target_sources(client PRIVATE client.cpp)
target_link_libraries(client PRIVATE qi::qi testlib)
