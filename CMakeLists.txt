cmake_minimum_required(VERSION 3.0)
project(bfjit)
add_executable(bfjit main.cxx)
target_compile_options(bfjit PRIVATE -O3 -g)
