cmake_minimum_required(VERSION 3.16)
project(blackhole LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# Use FetchContent for GLAD (you already have this in your build logs)
include(FetchContent)
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glad)        # target: glad

# NEW: find and link GLUT
find_package(GLUT REQUIRED)

add_executable(blackhole main.cpp)

# Link order: glad (loader) + OpenGL + GLUT (creates the window/context)
target_link_libraries(blackhole PRIVATE
  glad
  OpenGL::GL
  GLUT::GLUT
)
