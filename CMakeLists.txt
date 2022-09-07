cmake_minimum_required(VERSION 3.7)
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
project(sparse_distributed_representation)

option(STATIC_ANALYSIS "Run static code analysis" OFF)
option(BUILD_TESTING "Build tests and fuzzing" OFF)
option(CODE_COVERAGE "Create coverage stats. Needs lcov and (for clang) llvm-cov. Invoke with \"make cov-show\"" OFF)
set(PERF OFF CACHE STRING "Profile \"CPU\" or check the \"HEAP\". Needs libgoogle-perftools-dev. Invoke with \"make perf-show\"")
set_property(CACHE PERF PROPERTY STRINGS OFF CPU HEAP)
option(PGO "Build the fuzzer in two passes, the latter with profile guided optimization. Invoke with \"make pgo\"" OFF)

add_library(${PROJECT_NAME}_lib INTERFACE)

target_include_directories(${PROJECT_NAME}_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (STATIC_ANALYSIS)
  find_program(CLANGTIDY clang-tidy)
  if(CLANGTIDY)
    set(CMAKE_CXX_CLANG_TIDY 
    clang-tidy;
    "-checks=boost-*,\
      bugprone-*,\
      clang-analyzer-*,\
      cppcoreguidelines-*,\
      misc-*,\
      performance-*,\
      portability-*,\
      -*-magic-numbers,\
      -cppcoreguidelines-special-member-functions,\
      -cppcoreguidelines-pro-bounds-pointer-arithmetic,\
      -cppcoreguidelines-avoid-c-arrays,\
      -cppcoreguidelines-pro-type-const-cast,\
      -cppcoreguidelines-avoid-goto,\
      -cppcoreguidelines-init-variables,\
      -misc-non-private-member-variables-in-classes"
    --header-filter=${CMAKE_CURRENT_SOURCE_DIR}/include
    )
  else()
    message(SEND_ERROR "clang-tidy NOT FOUND")
  endif()
endif()

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

  enable_testing()
  add_test(NAME simple_tests COMMAND ./test_sdr)
  add_test(NAME fuzzy_tests COMMAND ./fuzz_sdr 20)

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
      COMMAND "CPUPROFILE=${CMAKE_CURRENT_BINARY_DIR}/cpuprofile" CPUPROFILE_FREQUENCY=1000000 ./fuzz_sdr 2>&1 1>/dev/null
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
      COMMAND HEAPCHECK=strict ./fuzz_sdr 2>&1 1>/dev/null
    )
  endif()

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

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      set(GCOV_TOOL_OPTION "--gcov-tool")
      set(GCOV_TOOL_VALUE "${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/llvm_gcov.sh")
    endif()

    add_custom_target(cov-proc
      # the tests must be run before the coverage can be processed
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/fuzz_sdr.dir/src/fuzz.cpp.gcda
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test_sdr.dir/src/test.cpp.gcda

      COMMAND lcov "${GCOV_TOOL_OPTION}" "${GCOV_TOOL_VALUE}" --capture --directory "${CMAKE_CURRENT_SOURCE_DIR}" --base-directory "${CMAKE_CURRENT_SOURCE_DIR}/include" --no-external --output-file "${CMAKE_CURRENT_BINARY_DIR}/coverage.info"
    )

    add_custom_target(cov-show
      DEPENDS cov-proc
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} # sensible-browser doesn't like path names in wsl for some reason
      COMMAND genhtml coverage.info
      COMMAND sensible-browser index.html
    )

  endif()

  if (PGO)
    if(CODE_COVERAGE OR NOT PERF STREQUAL "OFF")
      message(FATAL_ERROR "PGO must be run without other profiling options")
    endif()

    # for pgo, the executable needs to be build twice. once for getting the profile, the other for building with the profile
    # my goal is to have a single target automate the entire process, build, run, build again
    # I tried building the same executable again through a different target name (just copy pasting the same sources, etc),
    # but gcc didn't like that the gcda file was build by a different executable, and wouldn't accept it, "-Werror=coverage-mismatch"

    # in cmake you can't define the same executable target twice, so I'm left with this hacky workaround of running it manually
    # that's the purpose of the scripts in ./build_scripts, compiling and linking a target the exact same way cmake would do it
    add_custom_target(pgo-clean
      COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -type f -name '*.gcda' -delete
      COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -type f -name '*.profdata' -delete
      COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -type f -name '*.profraw' -delete
    )

    # override previous optimization level
    target_compile_options(fuzz_sdr PRIVATE -fprofile-generate)
    target_link_libraries(fuzz_sdr PRIVATE -fprofile-generate)

    add_custom_target(pgo
      DEPENDS fuzz_sdr
      COMMAND "${CMAKE_CURRENT_BINARY_DIR}/fuzz_sdr"

      # generate "build/compile_commands.json"
      COMMAND cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=true

      COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/pgo_proc_data.sh"

      # run the scipt that rebuilds the object file, but it instead reads the profile instead of generating it
      COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/pgo_compile.py" "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" "${CMAKE_CURRENT_SOURCE_DIR}/src/fuzz.cpp"

      # same as above but for linking
      COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/pgo_link.py" "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/fuzz_sdr.dir/link.txt"
    )

  endif()
  
endif()