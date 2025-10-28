cmake_minimum_required(VERSION 3.16)
project(blackhole LANGUAGES CXX)

# ---- language / warnings ----------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- deps: OpenGL, GLFW, GLAD, GLM -----------------------------------------
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)

include(FetchContent)
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glad)   # creates target: glad

# GLM is header-only; prefer system package if present
# (Ubuntu: sudo apt install libglm-dev)
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
if(NOT GLM_INCLUDE_DIR)
  message(STATUS "GLM not found system-wide; fetching headers...")
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
  )
  FetchContent_MakeAvailable(glm)
  # glm's include dir is <glm_SOURCE_DIR>
  set(GLM_INCLUDE_DIR ${glm_SOURCE_DIR})
endif()

# ---- sources ----------------------------------------------------------------
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/main.cpp"
)

add_executable(blackhole ${APP_SOURCES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR}/assets
)

target_include_directories(blackhole PRIVATE
  ${GLFW3_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}        
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(blackhole PRIVATE
  glad
  ${GLFW3_LIBRARIES}
  OpenGL::GL
)

# (Helpful on some pkg-config setups)
link_directories(${GLFW3_LIBRARY_DIRS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warnings (tweak as you like)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(blackhole PRIVATE -Wall -Wextra -Wpedantic)
elseif (MSVC)
  target_compile_options(blackhole PRIVATE /W4 /permissive-)
endif()
