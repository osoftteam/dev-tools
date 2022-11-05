cmake_minimum_required(VERSION 3.22)
add_compile_options(-g -Wall -Wextra -std=c++17 -O3)
project("cfix")
set(DEVUTILS_DIR "/home/ykhodak/dev-tools/dev/utils")

add_executable(cfix cfix_main.cpp cfix.cpp tcp_spinner.cpp)
target_include_directories(cfix PRIVATE ${DEVUTILS_DIR})
target_link_directories(cfix PRIVATE ${DEVUTILS_DIR}/build)
target_link_libraries(cfix ${GTEST_LIBRARIES} dev-utils)
