if(NOT BUILD_TESTING)
  return()
endif()

find_package(GTest REQUIRED)

find_package(qimodule REQUIRED)
find_package(
    qilang-tools REQUIRED
    PATHS "${PROJECT_SOURCE_DIR}/cmake"
    # Disable CMAKE_FIND_ROOT_PATH modes, when cross-compiling.
    NO_CMAKE_FIND_ROOT_PATH
)

# Generates this header to be able to test it
# (otherwise its content is injected in generated code).
unset(QILANG_COMMON_GEN_BEGIN)
unset(QILANG_COMMON_GEN_END)
configure_file(
    "${PROJECT_SOURCE_DIR}/qilang/gencodeutility.hpp.in"
    qilang/gencodeutility.hpp
    @ONLY
)

##############################################################################
# testqilang
# TestQiLang, a messy set of IDLs to check many cases
##############################################################################
add_library(testqilang SHARED)

set(
  testqilang_idl

  share/qi/idl/testqilang/buffy.idl.qi
  share/qi/idl/testqilang/someenums.idl.qi
  share/qi/idl/testqilang/someinterfaces.idl.qi
  share/qi/idl/testqilang/somemix.idl.qi
  share/qi/idl/testqilang/someproperties.idl.qi
  share/qi/idl/testqilang/somesignals.idl.qi
  share/qi/idl/testqilang/somestructs.idl.qi
  share/qi/idl/testqilang/time.idl.qi
)

qi_gen_idl(
  testqilang_generated
  CPP # Output language
  testqilang # Package name
  "${CMAKE_CURRENT_BINARY_DIR}" # Destination
  ${testqilang_idl} # IDL files
  IMPORT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}"
  NO_INSTALL
)

target_sources(
  testqilang
  PUBLIC
    testqilang/api.hpp
    ${testqilang_generated_INTERFACE}
    ${testqilang_generated_GMOCK}
  PRIVATE
    ${testqilang_generated_LOCAL}
    ${testqilang_generated_REMOTE}
    ${testqilang_idl}
)

target_include_directories(
  testqilang
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(
  testqilang
  PUBLIC
    qi::qi
)

##############################################################################
# testqilang_module
##############################################################################
qi_create_module(
  testqilang_module

  SRC
  src/anotherinterfaceimpl.hpp
  src/bradpitt.hpp
  src/buffy.hpp
  src/kindamanagerimpl.hpp
  src/ouroboros.hpp
  src/pingpong.hpp
  src/propertymaster.hpp
  src/registration.cpp
  src/signalmaster.hpp
  src/time.hpp
  src/truthseeker.hpp

  DEPENDS
  testqilang
  NO_INSTALL
)

target_include_directories(
  testqilang_module
  PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

##############################################################################
# testsubpackage
# Test qilang subpackages management
##############################################################################
qi_gen_lib(
  testsubpackage
  "${CMAKE_CURRENT_BINARY_DIR}" # Destination

  IDL
  share/qi/idl/testsubpackage/othersubpackage/othersubpackage.idl.qi
  share/qi/idl/testsubpackage/othersubpackage/localimport.idl.qi

  API_HEADER
  "testsubpackage/othersubpackage/api.hpp"

  DEPENDS
    testqilang
  IMPORT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}"
  NO_INSTALL
)

##############################################################################
# testimportpackage
# Test qilang package import feature
##############################################################################
qi_gen_lib(
  testimportpackage
  "${CMAKE_CURRENT_BINARY_DIR}" # Destination

  IDL
  share/qi/idl/testimportpackage/testimportpackage.idl.qi
  share/qi/idl/testimportpackage/importfromsubpackage.idl.qi

  DEPENDS
    testsubpackage
  IMPORT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}"
  NO_INSTALL
)

##############################################################################
# qisubpackage
# Test packages under "qi"
##############################################################################
add_library(qisubpackage SHARED)

qi_gen_idl(
  qisubpackage_generated
  CPP # Output language
  qi.subpackage # Package name
  "${CMAKE_CURRENT_BINARY_DIR}" # Destination
  share/qi/idl/qi/subpackage/mix.idl.qi # IDL files
  NO_INSTALL
)

target_include_directories(
  qisubpackage
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

target_sources(
  qisubpackage
  PUBLIC
    ${qisubpackage_generated_INTERFACE}
    ${qisubpackage_generated_GMOCK}
  PRIVATE
    ${qisubpackage_generated_LOCAL}
    ${qisubpackage_generated_REMOTE}
)
target_link_libraries(
  qisubpackage
  PUBLIC
    qi::qi
)

##############################################################################
# qisubpackageimpl
##############################################################################
add_library(qisubpackageimpl SHARED)

target_sources(
  qisubpackageimpl
  PRIVATE
    src/qi/subpackage/mix.hpp
    src/qi/subpackage/mix.cpp
)

target_link_libraries(
  qisubpackageimpl
  PUBLIC
    qisubpackage
)

##############################################################################
# test_qilang
# The actual entry point of the test
##############################################################################
add_executable(test_qilang)

target_sources(
  test_qilang
  PRIVATE
    test_qilang.hpp
    test_qilang.cpp
    test_qilang_raw.cpp
    test_qilang_enum_include.cpp
    test_qilang_function.cpp
    test_qilang_gmock.cpp
    test_qilang_import.cpp
    test_qilang_package.cpp
    test_qilang_property.cpp
    test_qilang_qisubpackage.cpp
    test_qilang_signal.cpp
    test_qilang_signature.cpp
    test_qilang_struct.cpp
    test_qilang_time.cpp
    test_qilang_type_registration.cpp
    test_qilang_gencodeutility.cpp
    test_qilang_test_utils.hpp
)

target_include_directories(
  test_qilang
  PRIVATE
    "${PROJECT_BINARY_DIR}"
)

target_link_libraries(
  test_qilang
  PRIVATE
    qilang
    qisubpackage
    qisubpackageimpl
    testimportpackage
    testqilang
    testsubpackage
    qi::qi
    qi::testsession
    GTest::gmock
)

add_test(
  NAME test_qilang
  COMMAND
    test_qilang
      --qi-sdk-prefix "${CMAKE_BINARY_DIR}/sdk"
)

set_tests_properties(
  test_qilang
  PROPERTIES
    TIMEOUT 90
)
