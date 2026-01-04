cmake_minimum_required(VERSION 3.1)
project(wonderland)

find_package(OpenGL REQUIRED)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_subdirectory(external)

include_directories(
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glad-opengl-3.3/include/
	external/tinygltf-2.9.3/
	external/
	wonderland/
)

add_executable(wonderland_window
	wonderland/wonderland_window.cpp
	wonderland/render/shader.cpp
)
target_link_libraries(wonderland_window
	${OPENGL_LIBRARY}
	glfw
	glad
)

add_executable(wonderland_redo
	wonderland/wonderland_redo.cpp
	wonderland/render/shader.cpp
)
target_link_libraries(wonderland_redo
	${OPENGL_LIBRARY}
	glfw
	glad
)
