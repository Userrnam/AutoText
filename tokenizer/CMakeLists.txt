cmake_minimum_required (VERSION 3.0)
project(tokenizer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)

add_library(tokenizer tokenizer.cpp tokenizer.hpp)
target_link_directories(tokenizer PUBLIC target/debug)
target_include_directories(tokenizer PUBLIC .)
target_link_libraries(tokenizer PUBLIC c_tokenizer)
