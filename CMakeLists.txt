cmake_minimum_required(VERSION 3.20)

############################################################
#                               Project
############################################################
project(SLURM_UTILS VERSION 0.1.0
  DESCRIPTION "Small custom collection of Slurm utilities that tabularize a lot of slurm outputs."
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

############################################################
#                               COnfigs
############################################################
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install prefix" FORCE)
endif()

# Adds the cmake utilities directory
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(GitSubmodules)

############################################################
#                               External packages
############################################################
# option(GIT_SUBMODULE "Check submodules during build" ON)
get_submodules() # needs git
check_submodule_exists(CLI11)
check_submodule_exists(json)

add_subdirectory(extern/CLI11)
add_subdirectory(extern/json)

option(WITH_JSON_INPUT "Enable JSON input via squeue --json (requires SLURM 21.08+)" OFF)

############################################################
#                               Targets
############################################################
add_executable(sstats
  apps/sstats.cpp
  src/util.cpp
)
target_include_directories(sstats PRIVATE include/)
target_link_libraries(sstats PRIVATE CLI11::CLI11)

# Link C++ runtime and libgcc statically so the binary runs on nodes
# with a different GCC version without needing to copy .so files over.
target_link_options(sstats PRIVATE -static-libstdc++ -static-libgcc)

if(WITH_JSON_INPUT)
  target_compile_definitions(sstats PRIVATE WITH_JSON_INPUT)
  target_link_libraries(sstats PRIVATE nlohmann_json::nlohmann_json)
endif()

install(TARGETS sstats RUNTIME DESTINATION bin)

install(CODE "
  string(FIND \"\$ENV{PATH}\" \"${CMAKE_INSTALL_PREFIX}/bin\" _path_idx)
  if(_path_idx EQUAL -1)
    message(STATUS \"\")
    message(STATUS \"Installed to ${CMAKE_INSTALL_PREFIX}/bin\")
    message(STATUS \"~/.local/bin is not in your PATH. Add this to your ~/.bashrc or ~/.zshrc:\")
    message(STATUS \"  export PATH=~/.local/bin:\\\$PATH\")
    message(STATUS \"\")
  else()
    message(STATUS \"Installed to ${CMAKE_INSTALL_PREFIX}/bin (already in PATH)\")
  endif()
")
