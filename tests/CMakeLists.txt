if(QI_WITH_TESTS)
  find_package(qimodule)
  qi_sanitize_compile_flags(HIDDEN_SYMBOLS)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  include_directories(".")

  # TestQiLang, a messy set of IDLs to check many cases
  set(
    testqilang_idl

    "testqilang/someenums.idl.qi"
    "testqilang/someinterfaces.idl.qi"
    "testqilang/somemix.idl.qi"
    "testqilang/someproperties.idl.qi"
    "testqilang/somesignals.idl.qi"
    "testqilang/somestructs.idl.qi"
    "testqilang/time.idl.qi"
  )

  qi_gen_idl(
    testqilang_generated
    CPP # Output language
    "testqilang" # Package name
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination
    ${testqilang_idl} # IDL files
  )

  qi_create_lib(
    testqilang SHARED

    "testqilang/api.hpp"
    ${testqilang_generated}
    ${testqilang_idl}

    DEPENDS
    qi
  )

  # Needed to ensure the build order
  add_dependencies(qi_gen_idl_testqilang qicc)

  qi_stage_lib(testqilang)

  qi_create_module(
    testqilang_module

    SRC
    "src/anotherinterfaceimpl.hpp"
    "src/bradpitt.hpp"
    "src/kindamanagerimpl.hpp"
    "src/ouroboros.hpp"
    "src/pingpong.hpp"
    "src/propertymaster.hpp"
    "src/registration.cpp"
    "src/signalmaster.hpp"
    "src/time.hpp"
    "src/truthseeker.hpp"

    DEPENDS
    qi
    testqilang
  )

  # Test qilang subpackages management
  set(testsubpackage_idl "testsubpackage/othersubpackage/othersubpackage.idl.qi")

  qi_gen_lib(
    testsubpackage
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination

    IDL
    ${testsubpackage_idl}

    API_HEADER
    "testsubpackage/othersubpackage/api.hpp"

    DEPENDS
    testqilang
  )

  # Needed to ensure the build order
  add_dependencies(qi_gen_idl_testsubpackage qicc)

  qi_stage_lib(testsubpackage)

  # Test qilang package import feature
  set(testimportpackage_idl "testimportpackage/testimportpackage.idl.qi")

  qi_gen_lib(
    testimportpackage
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination

    IDL
    ${testimportpackage_idl}

    DEPENDS
    testsubpackage
  )

  qi_stage_lib(testimportpackage)

  # Needed to ensure the build order
  add_dependencies(qi_gen_idl_testimportpackage qicc)

  # Test packages under "qi"
  set(qisubpackage_idl "qi/subpackage/mix.idl.qi")

  qi_gen_idl(
    qisubpackage_generated
    CPP # Output language
    qi.subpackage # Package name
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination
    ${qisubpackage_idl} # IDL files
  )

  qi_create_lib(
    qisubpackage SHARED

    ${qisubpackage_generated}

    DEPENDS
    qi
  )

  qi_stage_lib(qisubpackage)

  qi_create_lib(
    qisubpackageimpl SHARED

    "src/qi/subpackage/mix.hpp"
    "src/qi/subpackage/mix.cpp"

    DEPENDS
    qisubpackage
  )

  qi_stage_lib(qisubpackageimpl)

  # Needed to ensure the build order
  add_dependencies(qi_gen_idl_testimportpackage qicc)

  # The actual entry point of the test
  qi_create_gtest(
    test_qilang

    SRC
    "test_qilang.hpp"
    "test_qilang.cpp"
    "test_qilang_enum_include.cpp"
    "test_qilang_function.cpp"
    "test_qilang_package.cpp"
    "test_qilang_property.cpp"
    "test_qilang_qisubpackage.cpp"
    "test_qilang_signal.cpp"
    "test_qilang_signature.cpp"
    "test_qilang_struct.cpp"
    "test_qilang_time.cpp"
    "test_qilang_type_registration.cpp"

    DEPENDS
    qi
    qilang
    qisubpackage
    qisubpackageimpl
    testimportpackage
    testqilang
    testsubpackage

    TIMEOUT 10
  )
endif(QI_WITH_TESTS)
