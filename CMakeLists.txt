cmake_minimum_required(VERSION 3.20)
project(blackhole LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(USE_SYSTEM_GLFW "Use a system-installed GLFW instead of FetchContent" OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Allow very old cmake_minimum_required() in some third-party deps (e.g., older glad tags)
set(CMAKE_POLICY_VERSION_MINIMUM "3.5" CACHE STRING "Back-compat policy floor for deps")

# ------------------------------------------------------------------------------
# C++ standard
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Dependencies: OpenGL, GLFW, GLAD, GLM
# ------------------------------------------------------------------------------
# OpenGL (portable: links to OpenGL.framework on macOS, opengl32 on Windows, libGL on Linux)
find_package(OpenGL REQUIRED)

include(FetchContent)

# --- GLAD (OpenGL loader) -----------------------------------------------------
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glad)   # targets: glad
# Some tags don't export include dirs as an interface; add explicitly for safety.
set(GLAD_INCLUDE_DIR "${glad_SOURCE_DIR}/include")

# --- GLM (header-only) --------------------------------------------------------
# Prefer system include if already installed; otherwise fetch.
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
if(NOT GLM_INCLUDE_DIR)
  message(STATUS "GLM not found system-wide; fetching headers...")
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
  )
  FetchContent_MakeAvailable(glm)
  set(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}")
endif()

# --- GLFW ---------------------------------------------------------------------
set(GLFW_TARGET glfw)
set(GLFW_INCLUDE_DIRS "")
set(GLFW_LIBRARIES "")

if(USE_SYSTEM_GLFW)
  # Try config package first (Windows vcpkg/Homebrew formula installs this)
  find_package(glfw3 CONFIG QUIET)
  if(glfw3_FOUND)
    set(GLFW_TARGET glfw)
  else()
    # Fallback: pkg-config (common on Linux/macOS)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
      pkg_check_modules(GLFW3 QUIET glfw3)
    endif()
    if(GLFW3_FOUND)
      # We'll link using imported target we define here
      add_library(GLFW3::GLFW3 INTERFACE IMPORTED)
      set_target_properties(GLFW3::GLFW3 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW3_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES      "${GLFW3_LIBRARIES}"
      )
      set(GLFW_TARGET GLFW3::GLFW3)
      set(GLFW_INCLUDE_DIRS "${GLFW3_INCLUDE_DIRS}")
      set(GLFW_LIBRARIES "${GLFW3_LIBRARIES}")
    else()
      message(FATAL_ERROR "USE_SYSTEM_GLFW=ON but no system GLFW was found. "
                          "Install glfw3 or configure with -DUSE_SYSTEM_GLFW=OFF to build it from source.")
    endIf()
  endif()
else()
  # Fetch & build GLFW from source (portable)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
  )
  FetchContent_MakeAvailable(glfw)  # targets: glfw
endif()

# ------------------------------------------------------------------------------
# Sources / Target
# ------------------------------------------------------------------------------
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/main.cpp"
)

add_executable(${PROJECT_NAME} ${APP_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
  ${GLAD_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIRS}      # may be empty; harmless
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
  glad
  ${GLFW_TARGET}
  OpenGL::GL
)

# macOS specifics are handled by GLFW's own target (Cocoa, IOKit, CoreVideo).
# On Linux it links X11/Wayland as needed. On Windows it links appropriate libs.

# ------------------------------------------------------------------------------
# Post-build: copy assets next to the binary
# ------------------------------------------------------------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets"
          "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
  VERBATIM
)

# ------------------------------------------------------------------------------
# Warnings
# ------------------------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
endif()
