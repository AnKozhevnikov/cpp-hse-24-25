cmake_minimum_required(VERSION 3.8)
project(SearchingTree)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

file(GLOB SEARCHINGTREE_SRC ${CMAKE_SOURCE_DIR}/include/*.*)

add_library(SearchingTree INTERFACE)
target_include_directories(SearchingTree INTERFACE ${CMAKE_SOURCE_DIR}/include)

add_executable(demo demo.cpp)
target_link_libraries(demo SearchingTree)