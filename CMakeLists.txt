cmake_minimum_required(VERSION 3.18.4...3.21.1)

project(BTIDG2 LANGUAGES C)

# MPI
find_package(MPI REQUIRED)

include(GNUInstallDirs)

add_library(btidg2 INTERFACE)

target_include_directories( btidg2 INTERFACE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/includes>"
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> 
)

target_link_libraries( btidg2 INTERFACE MPI::MPI_C)

add_subdirectory(example)

option(BUILD_WITH_DOCS "BUILD with documentation" OFF)

if (BUILD_WITH_DOCS)
	add_subdirectory(docs)
endif()