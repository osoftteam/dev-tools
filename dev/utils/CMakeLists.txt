cmake_minimum_required(VERSION 3.22)
add_compile_options(-g -Wall -Wextra -std=c++17 -O3)
project(dev-utils)
add_library(dev-utils
  tag_generator.cpp
  dev-utils.cpp
  ctf-messenger.cpp
  fix-utils.cpp
  skt-utils.cpp)
