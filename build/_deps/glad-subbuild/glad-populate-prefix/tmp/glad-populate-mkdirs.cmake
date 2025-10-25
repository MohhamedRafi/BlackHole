# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/mohha/projects/blackhole_project/build/_deps/glad-src"
  "/home/mohha/projects/blackhole_project/build/_deps/glad-build"
  "/home/mohha/projects/blackhole_project/build/_deps/glad-subbuild/glad-populate-prefix"
  "/home/mohha/projects/blackhole_project/build/_deps/glad-subbuild/glad-populate-prefix/tmp"
  "/home/mohha/projects/blackhole_project/build/_deps/glad-subbuild/glad-populate-prefix/src/glad-populate-stamp"
  "/home/mohha/projects/blackhole_project/build/_deps/glad-subbuild/glad-populate-prefix/src"
  "/home/mohha/projects/blackhole_project/build/_deps/glad-subbuild/glad-populate-prefix/src/glad-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/mohha/projects/blackhole_project/build/_deps/glad-subbuild/glad-populate-prefix/src/glad-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/mohha/projects/blackhole_project/build/_deps/glad-subbuild/glad-populate-prefix/src/glad-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
