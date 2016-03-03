if(QI_WITH_TESTS)
  find_package(qimodule)

  set(
    test_qilang_idl_files

    "testqilang/testinterface.idl.qi"
  )

  # Generate code from qilang idl files
  qi_gen_idl(
    test_qilang_idl
    CPP # Output language
    "testqilang" # Package name
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination
    ${test_qilang_idl_files} # IDL files
  )

  # Create a library to reuse the generated code
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  include_directories(".")
  qi_create_lib(
    testqilang

    "testqilang/api.hpp"
    ${test_qilang_idl_INTERFACE}
    ${test_qilang_idl_LOCAL}
    ${test_qilang_idl_REMOTE}
    ${test_qilang_idl_files}

    DEPENDS
    qi
  )

  qi_stage_lib(testqilang)

  qi_create_module(
    testqilang_module

    SRC
    "src/kindamanagerimpl.hpp"
    "src/anotherinterfaceimpl.hpp"
    "src/ouroboros.hpp"
    "src/registration.cpp"

    DEPENDS
    qi
    testqilang
  )

endif(QI_WITH_TESTS)

qi_create_gtest(
  test_qilang

  SRC
  "test_qilang.cpp"
  "test_qilang_signature.cpp"
  "test_qilang_type_registration.cpp"

  DEPENDS
  qi
  qilang
  testqilang

  TIMEOUT 10
)
