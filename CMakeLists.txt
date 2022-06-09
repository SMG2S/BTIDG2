cmake_minimum_required(VERSION 3.18.4...3.21.1)

project(GtwoML)


# MPI
find_package(MPI REQUIRED)

add_subdirectory(example)


option(BUILD_WITH_DOCS "BUILD with documentation" OFF)

if (BUILD_WITH_DOCS)
	add_subdirectory(docs)
endif()