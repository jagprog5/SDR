cmake_minimum_required(VERSION 3.7)
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
project(sparse_distributed_representation)

option(BUILD_TESTING "Build tests and fuzzing" OFF)
option(CODE_COVERAGE "Create coverage stats. Needs lcov and (for clang) llvm-cov. Invoke with \"make cov-show\"" OFF)
set(PERF OFF CACHE STRING "Profile \"CPU\" or check the \"HEAP\". Needs libgoogle-perftools-dev. Invoke with \"make perf-show\"") 
set_property(CACHE PERF PROPERTY STRINGS OFF CPU HEAP)

add_library(${PROJECT_NAME}_lib INTERFACE)

target_include_directories(${PROJECT_NAME}_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(BUILD_TESTING)
  # fuzzer for the complicated functions
  add_executable(fuzz_sdr src/fuzz.cpp)
  target_compile_options(fuzz_sdr PRIVATE -Wall -Wextra -Ofast)
  target_link_libraries(fuzz_sdr PRIVATE ${PROJECT_NAME}_lib)
  
  if (CODE_COVERAGE AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # prevent a crash with clang ¯\_(ツ)_/¯
    set(Boost_USE_STATIC_LIBS ON)
  endif()
  
  # unit tests
  set(BOOST_MIN_VERSION "1.63.0")
  find_package(Boost REQUIRED COMPONENTS unit_test_framework)
  add_executable(test_sdr src/test.cpp)
  target_compile_options(test_sdr PRIVATE -Wall -Wextra -Ofast)
  target_link_libraries(test_sdr PRIVATE ${PROJECT_NAME}_lib ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

  if(NOT PERF STREQUAL "OFF")
    # https://stackoverflow.com/a/49104701/15534181
    target_link_options(fuzz_sdr PUBLIC -Wl,-no-as-needed)
  endif()

  if (PERF STREQUAL "CPU")
    find_library(PROFILER profiler REQUIRED)
    if (NOT PROFILER)
      message(FATAL_ERROR "profiler not found")
    endif()
    target_link_libraries(fuzz_sdr PRIVATE ${PROFILER})

    add_custom_target(perf-show
      # https://gperftools.github.io/gperftools/cpuprofile.html
      DEPENDS fuzz_sdr
      COMMAND "CPUPROFILE=${CMAKE_CURRENT_BINARY_DIR}/cpuprofile" CPUPROFILE_FREQUENCY=1000000 ./fuzz_sdr 200 2>&1 1>/dev/null
      COMMAND google-pprof --web fuzz_sdr "${CMAKE_CURRENT_BINARY_DIR}/cpuprofile"
    )
  elseif(PERF STREQUAL "HEAP")
    find_library(TCMALLOC tcmalloc REQUIRED)
    if (NOT TCMALLOC)
      message(FATAL_ERROR "tcmalloc not found")
    endif()
    target_link_libraries(fuzz_sdr PRIVATE ${TCMALLOC})

    add_custom_target(perf-show
      DEPENDS fuzz_sdr
      COMMAND HEAPCHECK=strict ./fuzz_sdr 50 2>&1 1>/dev/null
    )
  endif()

  enable_testing()
  add_test(NAME simple_tests COMMAND ./test_sdr)
  add_test(NAME fuzzy_tests COMMAND ./fuzz_sdr 20)

  if (CODE_COVERAGE)
    # enable coverage for the test and fuzz targets
    # -O0 overrides previously stated optimization level. for better debugging
    target_compile_options(fuzz_sdr PRIVATE -O0 -g --coverage)
    target_compile_options(test_sdr PRIVATE -O0 -g --coverage)
    target_link_libraries(fuzz_sdr PRIVATE --coverage)
    target_link_libraries(test_sdr PRIVATE --coverage)

    add_custom_target(cov-clean
      COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -type f -name '*.gcda' -delete
      COMMAND rm -f "${CMAKE_CURRENT_BINARY_DIR}/coverage.info"
      COMMAND rm -f "${CMAKE_CURRENT_BINARY_DIR}/*.png"
      COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -type f -name '*.html' -delete
      COMMAND rm -f "${CMAKE_CURRENT_BINARY_DIR}/gcov.css"
    )

    # used to make cov-proc run the tests if they haven't yet been run
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/fuzz_sdr.dir/src/fuzz.cpp.gcda
      DEPENDS fuzz_sdr
      COMMAND ctest -j "${CMAKE_CURRENT_BINARY_DIR}"
    )

    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_sdr.dir/src/test.cpp.gcda
      DEPENDS test_sdr
      COMMAND ctest -j "${CMAKE_CURRENT_BINARY_DIR}"
    )

    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set(GCOV_TOOL_OPTION "--gcov-tool")
      set(GCOV_TOOL_VALUE "${CMAKE_CURRENT_SOURCE_DIR}/llvm-gcov.sh")
    endif()

    add_custom_target(cov-proc
      # the tests must be run before the coverage can be processed
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/fuzz_sdr.dir/src/fuzz.cpp.gcda
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_sdr.dir/src/test.cpp.gcda

      COMMAND lcov "${GCOV_TOOL_OPTION}" "${GCOV_TOOL_VALUE}" --capture --directory "${CMAKE_CURRENT_SOURCE_DIR}" --base-directory "${CMAKE_CURRENT_SOURCE_DIR}/include" --no-external --output-file "${CMAKE_CURRENT_BINARY_DIR}/coverage.info"
    )

    add_custom_target(cov-show
      DEPENDS cov-proc
      COMMAND genhtml "${CMAKE_CURRENT_BINARY_DIR}/coverage.info"
      COMMAND xdg-open "${CMAKE_CURRENT_BINARY_DIR}/index.html"
    )

  endif()
endif()