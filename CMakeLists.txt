cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 17)
project(sparse_distributed_representation)

add_library(${PROJECT_NAME}_lib INTERFACE)

target_include_directories(${PROJECT_NAME}_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(BUILD_TESTING "Build tests and fuzzing" OFF)
option(CODE_COVERAGE "Create coverage stats for tests. Requires BUILD_TESTING=true and g++" OFF)

if(BUILD_TESTING)
  # fuzzer for the complicated functions
  add_executable(fuzz_sdr src/fuzz.cpp)
  target_compile_options(fuzz_sdr PRIVATE -Wall -Wextra -Ofast)
  target_link_libraries(fuzz_sdr PRIVATE ${PROJECT_NAME}_lib)
  
  # unit tests
  set(BOOST_MIN_VERSION "1.63.0")
  find_package(Boost REQUIRED COMPONENTS unit_test_framework)
  add_executable(test_sdr src/test.cpp)
  target_compile_options(test_sdr PRIVATE -Wall -Wextra -Ofast)
  target_link_libraries(test_sdr PRIVATE ${PROJECT_NAME}_lib ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

  enable_testing()
  add_test(NAME simple_tests COMMAND ./test_sdr)
  add_test(NAME fuzzy_tests COMMAND ./fuzz_sdr 20)

  if (CODE_COVERAGE)
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      message(FATAL_ERROR "coverage requires g++")
    else()
      # enable coverage for the test and fuzz targets
      # -O0 overrides previously stated optimization level
      target_compile_options(fuzz_sdr PRIVATE -O0 -g --coverage)
      target_compile_options(test_sdr PRIVATE -O0 -g --coverage)
      target_link_libraries(fuzz_sdr PRIVATE -lgcov)
      target_link_libraries(test_sdr PRIVATE -lgcov)
    endif()
  endif()
endif()