if(QI_WITH_TESTS)
  find_package(qimodule)
  qi_sanitize_compile_flags(HIDDEN_SYMBOLS)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  include_directories(".")

  # TestQiLang, a messy set of IDLs to check many cases
  set(
    testqilang_idl

    "testqilang/somestructs.idl.qi"
    "testqilang/somemix.idl.qi"
  )

  qi_gen_idl(
    testqilang_generated
    CPP # Output language
    "testqilang" # Package name
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination
    ${testqilang_idl} # IDL files
  )

  qi_create_lib(
    testqilang

    "testqilang/api.hpp"
    ${testqilang_generated}
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
    "src/bradpitt.hpp"
    "src/ouroboros.hpp"
    "src/pingpong.hpp"
    "src/registration.cpp"

    DEPENDS
    qi
    testqilang
  )

  # Test qilang subpackages management
  set(testsubpackage_idl "testsubpackage/othersubpackage/othersubpackage.idl.qi")

  qi_gen_idl(
    testsubpackage_generated
    CPP # Output language
    "testsubpackage" # Package name
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination
    ${testsubpackage_idl} # IDL files
  )

  qi_create_lib(
    testsubpackage

    "testsubpackage/othersubpackage/api.hpp"
    ${testsubpackage_generated}
    ${testsubpackage_idl}

    DEPENDS
    qi
    testqilang
  )

  qi_stage_lib(testsubpackage)

  # Test qilang package import feature
  set(testimportpackage_idl "testimportpackage/testimportpackage.idl.qi")

  qi_gen_idl(
    testimportpackage_generated
    CPP # Output language
    "testimportpackage" # Package name
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination
    ${testimportpackage_idl} # IDL files
  )

  qi_create_lib(
    testimportpackage

    "testimportpackage/api.hpp"
    ${testimportpackage_generated}
    ${testimportpackage_idl}

    DEPENDS
    qi
    testsubpackage
  )

  qi_stage_lib(testimportpackage)

  # The actual entry point of the test
  qi_create_gtest(
    test_qilang

    SRC
    "test_qilang.cpp"
    "test_qilang_function.cpp"
    "test_qilang_signature.cpp"
    "test_qilang_type_registration.cpp"
    "test_qilang_package.cpp"
    "test_qilang_struct_include.cpp"

    DEPENDS
    qi
    qilang
    testqilang
    testsubpackage
    testimportpackage

    TIMEOUT 10
  )


endif(QI_WITH_TESTS)
