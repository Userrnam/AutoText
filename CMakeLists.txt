cmake_minimum_required(VERSION 3.0)
project(AutoText LANGUAGES CXX)

add_subdirectory(rwkv.cpp)
add_subdirectory(tokenizer)

set(CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME}
	src/main.cpp
	src/Model.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE rwkv.cpp tokenizer)
target_link_libraries(${PROJECT_NAME} PRIVATE rwkv tokenizer)
