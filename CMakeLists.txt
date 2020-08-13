cmake_minimum_required( VERSION 3.0 )

project( d2m )

add_subdirectory(lib)
add_subdirectory(dicom2mesh)

option(BUILD_GUI "Build Dicom2Mesh GUI" OFF)
if(BUILD_GUI)
    add_subdirectory(gui)
endif(BUILD_GUI)

option(GCOV "COVERAGE" OFF)
IF(GCOV)
    message(STATUS "COVERAGE ON")
    set(COVERAGE_COMPILER_FLAGS "-fprofile-arcs -ftest-coverage -O0" CACHE INTERNAL "")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILER_FLAGS}")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILER_FLAGS}")
ENDIF()


