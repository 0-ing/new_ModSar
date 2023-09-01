
## How to use gtest as test framework in ModSar

#### 0x01 GTest Version

Currently, GTest Version **1.14.0** is integrated in project by head files and library.

#### 0x02 How to use GTest in compile

Take helloworld project in examples directory as an example.

In textbox below, you can find the directory structure for the basic example.

```bash
$ tree examples/helloworld
examples/helloworld
├── CMakeLists.txt
├── inc
│   └── sum.h
├── src
│   ├── main.cpp
│   └── sum.cpp
└── tests
    └── test_sum.cpp
```

Test source code are under `tests`. Compile commands are in CMakelists.txt.

```cmake
include_directories(
    ${EXTERNEL_INSTALL}/include
    ${PACKAGE_SOURCE_ROOT}
    ${PACKAGE_SOURCE_ROOT}/inc
    )
link_directories(${EXTERNEL_INSTALL}/lib)
```

Include directories and lib directories are specified by `include_directories` and `link_directories`. The variable `EXTERNEL_INSTALL` is set by buildCentral, value is `<path to ModSar>/external/install`, which is include files and shared library files position.

```cmake
enable_testing()

set(GTEST_BIN hello_test)
file(GLOB HELLOWORLD_TESTS_SRC
    "${PACKAGE_SOURCE_ROOT}/tests/*.cpp"
    "${PACKAGE_SOURCE_ROOT}/src/sum.cpp"
)

add_executable(${GTEST_BIN} ${HELLOWORLD_TESTS_SRC})

target_link_libraries(${GTEST_BIN} gtest pthread)

include(GoogleTest)
gtest_discover_tests(${GTEST_BIN})
# install(TARGETS hello_test DESTINATION ${BUILD_OUT})

add_custom_target(RUN_Gtest ALL DEPENDS ${GTEST_BIN})
add_custom_command(TARGET RUN_Gtest
    POST_BUILD
    COMMAND ${GTEST_BIN} --gtest ${GTEST_BIN} --gtest_output=xml:${BUILD_OUT}/../lcov/gtest/${GTEST_BIN}.xml --gtest_color=yes || echo "Gtest Failed,but continue next step"
)
```

The above configuration enables testing in CMake, declares the `hello_test` binary, and links it to `gtest`. The `include(GoogleTest)` and `gtest_discover_tests(${GTEST_BIN})` enable Cmake's test runner to discover the tests included in the binary. You can run `ctest` in buildCentral output directory. The last two lines add custom command at post-build stage, run the test binary (`hello_test`) after compile, and generate test result.

#### Reference


* [Quickstart: Building with CMake](https://google.github.io/googletest/quickstart-cmake.html)

