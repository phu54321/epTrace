cmake_minimum_required (VERSION 2.8)
project (epTrace)

include_directories(../../Contrib)

add_executable(epTrace  epTrace.cpp)
target_link_libraries(epTrace BlackBone)
