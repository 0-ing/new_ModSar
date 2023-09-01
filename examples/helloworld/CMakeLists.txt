
#################################################################################
#                  common parts for all cmake scripts                           #
#            just copy as the beginning of root CMakeLists.txt                  #
#################################################################################

cmake_minimum_required(VERSION 3.1.3)
project(${PACKAGE_NAME} C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (DEFINED RULE_DIR)
    include(${RULE_DIR}/rule_base.cmake)
else()
    set(RULE_DIR ${CMAKE_SOURCE_DIR})
    include(rule_base.cmake)
endif()

set(PACKAGE_SOURCE_ROOT ${CMAKE_SOURCE_DIR})


print_variable(PROJECT_ROOT)
print_variable(EXTERNEL_INSTALL)
print_variable(PACKAGE_SOURCE_ROOT)
print_variable(BUILD_OUT)

#################################################################################
#                cmake script for project-specific package                      #
#################################################################################

include_directories(
    ${EXTERNEL_INSTALL}/include
    ${PACKAGE_SOURCE_ROOT}
    ${PACKAGE_SOURCE_ROOT}/inc
    )
link_directories(${EXTERNEL_INSTALL}/lib)



file(GLOB HELLOWORLD_SRC "${PACKAGE_SOURCE_ROOT}/src/*.cpp")

add_executable(helloworld ${HELLOWORLD_SRC})

install(TARGETS helloworld DESTINATION ${BUILD_OUT})


#################################################################################
#                cmake script for project-specific package                      #
#################################################################################

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
