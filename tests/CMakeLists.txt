if(QI_WITH_TESTS)
  find_package(qimodule)
  qi_sanitize_compile_flags(HIDDEN_SYMBOLS)
  include_directories(".")
  qi_stage_dir(".")

  # Below, we were lazy to make the distribution of these tests correctly in deploys.
  # It was preferred to simply prevent those files from being installed, and instead only
  # use them locally to the build.
  # TODO: allow modules for test only, and improve qi_gen_lib to allow to run these tests from
  # a deploy.

  # TestQiLang, a messy set of IDLs to check many cases
  set(
    testqilang_idl

    "share/qi/idl/testqilang/buffy.idl.qi"
    "share/qi/idl/testqilang/someenums.idl.qi"
    "share/qi/idl/testqilang/someinterfaces.idl.qi"
    "share/qi/idl/testqilang/somemix.idl.qi"
    "share/qi/idl/testqilang/someproperties.idl.qi"
    "share/qi/idl/testqilang/somesignals.idl.qi"
    "share/qi/idl/testqilang/somestructs.idl.qi"
    "share/qi/idl/testqilang/time.idl.qi"
  )

  qi_gen_idl(
    testqilang_generated
    CPP # Output language
    "testqilang" # Package name
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination
    ${testqilang_idl} # IDL files
    NO_INSTALL
  )

  qi_create_lib(
    testqilang SHARED

    "testqilang/api.hpp"
    ${testqilang_generated}
    ${testqilang_idl}

    DEPENDS
    qi
    NO_INSTALL
  )

  # Needed to ensure the build order
  add_dependencies(qi_gen_idl_testqilang qicc)

  qi_stage_lib(testqilang)

  qi_create_module(
    testqilang_module

    SRC
    "src/anotherinterfaceimpl.hpp"
    "src/bradpitt.hpp"
    "src/buffy.hpp"
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
    NO_INSTALL
  )

  # Test qilang subpackages management
  set(
    testsubpackage_idl

    "share/qi/idl/testsubpackage/othersubpackage/othersubpackage.idl.qi"
    "share/qi/idl/testsubpackage/othersubpackage/localimport.idl.qi"
  )

  qi_gen_lib(
    testsubpackage
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination

    IDL
    ${testsubpackage_idl}

    API_HEADER
    "testsubpackage/othersubpackage/api.hpp"

    DEPENDS
    testqilang
    NO_INSTALL
  )

  # Needed to ensure the build order
  add_dependencies(qi_gen_idl_testsubpackage qicc)

  qi_stage_lib(testsubpackage)

  # Test qilang package import feature
  set(
    testimportpackage_idl

    "share/qi/idl/testimportpackage/testimportpackage.idl.qi"
    "share/qi/idl/testimportpackage/importfromsubpackage.idl.qi"
  )

  qi_gen_lib(
    testimportpackage
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination

    IDL
    ${testimportpackage_idl}

    DEPENDS
    testsubpackage
    NO_INSTALL
  )

  qi_stage_lib(testimportpackage)

  # Needed to ensure the build order
  add_dependencies(qi_gen_idl_testimportpackage qicc)

  # Test packages under "qi"
  set(qisubpackage_idl "share/qi/idl/qi/subpackage/mix.idl.qi")

  qi_gen_idl(
    qisubpackage_generated
    CPP # Output language
    qi.subpackage # Package name
    "${CMAKE_CURRENT_BINARY_DIR}" # Destination
    ${qisubpackage_idl} # IDL files
    NO_INSTALL
  )

  qi_create_lib(
    qisubpackage SHARED

    ${qisubpackage_generated}

    DEPENDS
    qi
    NO_INSTALL
  )

  qi_stage_lib(qisubpackage)

  qi_create_lib(
    qisubpackageimpl SHARED

    "src/qi/subpackage/mix.hpp"
    "src/qi/subpackage/mix.cpp"

    DEPENDS
    qisubpackage
    NO_INSTALL
  )

  qi_stage_lib(qisubpackageimpl)

  # Needed to ensure the build order
  add_dependencies(qi_gen_idl_testimportpackage qicc)

  # The actual entry point of the test
  qi_create_gmock(
    test_qilang

    SRC
    "test_qilang.hpp"
    "test_qilang.cpp"
    "test_qilang_raw.cpp"
    "test_qilang_enum_include.cpp"
    "test_qilang_function.cpp"
    "test_qilang_gmock.cpp"
    "test_qilang_import.cpp"
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
    testsession
    qitestutils

    TIMEOUT 10
    NO_INSTALL
  )
endif(QI_WITH_TESTS)
