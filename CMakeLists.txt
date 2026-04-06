cmake_minimum_required(VERSION 3.20)
project(SLURM_UTILS)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_library(slurm_utils STATIC
  src/utils.cpp
  src/jobs.cpp
  src/accounts.cpp
  src/users.cpp
)
target_include_directories(slurm_utils PUBLIC include/)

add_executable(saccstats src/saccstats.cpp)
target_link_libraries(saccstats PRIVATE slurm_utils)

add_executable(suserstats src/susersstats.cpp)
target_link_libraries(suserstats PRIVATE slurm_utils)
