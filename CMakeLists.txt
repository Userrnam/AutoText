cmake_minimum_required(VERSION 3.0)
project(AutoText LANGUAGES CXX)

add_subdirectory(glfw)
add_subdirectory(rwkv.cpp)
add_subdirectory(tokenizer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

# imgui
add_library(imgui
	imgui/imgui_draw.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_demo.cpp
	imgui/imgui.cpp
	imgui/backends/imgui_impl_glfw.cpp
	imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC imgui imgui/backends)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(imgui PUBLIC ${OPENGL_LIBRARIES} glfw)

# AutoText
add_compile_definitions(PROJECT_PATH="${CMAKE_SOURCE_DIR}")
add_executable(${PROJECT_NAME}
	src/main.cpp
	src/Model.cpp
	src/Application.cpp
	src/TextGenerator.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE rwkv tokenizer imgui)
