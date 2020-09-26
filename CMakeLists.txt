add_library(CppUTest
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
)

if(NOT CPP_PLATFORM)
  message(FATAL_ERROR "Using CppUTest requires definition of CPP_PLATFORM")
else()
  target_sources(CppUTest PRIVATE platforms/${CPP_PLATFORM}/UtestPlatform.cpp)
endif()

target_include_directories(CppUTest PUBLIC include)
target_link_libraries(CppUTest PRIVATE prj_device_target)
target_compile_definitions(CppUTest PUBLIC
  CPPUTEST_MEM_LEAK_DETECTION_DISABLED  # Requires exceptions to enable
  CPPUTEST_STD_CPP_LIB_DISABLED         # Requires exceptions to enable
)
export(TARGETS CppUTest FILE "${PROJECT_BINARY_DIR}/CppUTest/CppUTest.cmake")