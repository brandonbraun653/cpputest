include("${COMMON_TOOL_ROOT}/cmake/utility/embedded.cmake")

gen_intf_lib(
  TARGET
    CppUTest_inc
  INTF_INCLUDES
    "./include"
  INTF_DEFINES
    CPPUTEST_MEM_LEAK_DETECTION_DISABLED
    CPPUTEST_STD_CPP_LIB_DISABLED
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/CommonTools/CppUTest"
)

gen_static_lib_variants(
  TARGET
    CppUTest
  SOURCES
    src/CppUTest/CommandLineArguments.cpp
    src/CppUTest/MemoryLeakWarningPlugin.cpp
    src/CppUTest/TestHarness_c.cpp
    src/CppUTest/TestRegistry.cpp
    src/CppUTest/CommandLineTestRunner.cpp
    src/CppUTest/SimpleString.cpp
    src/CppUTest/SimpleStringInternalCache.cpp
    src/CppUTest/TestMemoryAllocator.cpp
    src/CppUTest/TestResult.cpp
    src/CppUTest/JUnitTestOutput.cpp
    src/CppUTest/TeamCityTestOutput.cpp
    src/CppUTest/TestFailure.cpp
    src/CppUTest/TestOutput.cpp
    src/CppUTest/MemoryLeakDetector.cpp
    src/CppUTest/TestFilter.cpp
    src/CppUTest/TestPlugin.cpp
    src/CppUTest/TestTestingFixture.cpp
    src/CppUTest/SimpleMutex.cpp
    src/CppUTest/Utest.cpp
  PUB_LIBRARIES
    CppUTest_inc
    prj_device_target
    prj_build_target
  EXPORT_DIR
    "${PROJECT_BINARY_DIR}/CommonTools/CppUTest"
)

if(CPP_PLATFORM)
  target_sources(CppUTest PRIVATE ${COMMON_TOOL_ROOT}/cpputest/platforms/${CPP_PLATFORM}/UTestPlatform.cpp)
  target_link_libraries(CppUTest PRIVATE aurora_intf_inc chimera_intf_inc)
else()
  target_sources(CppUTest PRIVATE ${COMMON_TOOL_ROOT}/cpputest/platforms/gcc/UTestPlatform.cpp)
endif()
