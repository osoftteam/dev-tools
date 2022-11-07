cmake_minimum_required(VERSION 3.22)

set(CMAKE_VERBOSE_MAKEFILE ON)
add_compile_options(-Wall -Wextra -std=c++17 -O3)

project("dev-test")
find_package(GTest REQUIRED)
set(DEVUTILS_DIR "~/dev-tools/dev/utils")


add_executable(test_add test_add.cpp)
target_link_libraries(test_add ${GTEST_LIBRARIES})

add_executable(test_atoi test_atoi.cpp)
target_include_directories(test_atoi PRIVATE ${DEVUTILS_DIR})
target_link_directories(test_atoi PRIVATE ${DEVUTILS_DIR}/build)
target_link_libraries(test_atoi ${GTEST_LIBRARIES} dev-utils)

add_executable(test_tag_generator test_tag_generator.cpp)
target_include_directories(test_tag_generator PRIVATE ${DEVUTILS_DIR})
target_link_directories(test_tag_generator PRIVATE ${DEVUTILS_DIR}/build)
target_link_libraries(test_tag_generator ${GTEST_LIBRARIES} dev-utils)

