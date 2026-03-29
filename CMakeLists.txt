cmake_minimum_required(VERSION ${CMAKE_VERSION})
set(CMAKE_CXX_STANDARD 20)

project(libngraphs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

# if built independently build the demo please!
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  add_subdirectory(demo)
endif()
