if(QI_WITH_TESTS)
  find_package(qimodule)

  set(
    testqilang_idl

    "testqilang/somestructs.idl.qi"
    "testqilang/somemix.idl.qi"
  )

  # Generate code from qilang idl files
  qi_gen_idl(
    testqilang_generated
    CPP # Output language
    "testqilang" # Package name
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination
    ${testqilang_idl} # IDL files
  )

  # Create a library to reuse the generated code
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  include_directories(".")

  qi_create_lib(
    testqilang

    "testqilang/api.hpp"
    ${testqilang_generated_INTERFACE}
    ${testqilang_generated_LOCAL}
    ${testqilang_generated_REMOTE}
    ${testqilang_idl}

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
    "src/pingpong.hpp"
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
  "test_qilang_struct_include.cpp"

  DEPENDS
  qi
  qilang
  testqilang

  TIMEOUT 10
)
