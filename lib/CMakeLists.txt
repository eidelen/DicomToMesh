cmake_minimum_required( VERSION 3.0 )

project( dicom2meshlib )

find_package( VTK REQUIRED )
set(DICOM_TO_MESH_LIB_LIBS ${VTK_LIBRARIES})

FILE(GLOB  DICOM_TO_MESH_LIB_INC       inc/*.h)
FILE(GLOB  DICOM_TO_MESH_LIB_SRC       src/*.cpp)
set(DICOM_TO_MESH_INC_DIRS inc)

include_directories( ${DICOM_TO_MESH_INC_DIRS} )

ADD_LIBRARY(dicom2meshlib STATIC ${DICOM_TO_MESH_LIB_INC} ${DICOM_TO_MESH_LIB_SRC})
target_link_libraries(dicom2meshlib ${DICOM_TO_MESH_LIB_LIBS})
target_include_directories(dicom2meshlib INTERFACE ${DICOM_TO_MESH_INC_DIRS})
target_compile_features(dicom2meshlib PRIVATE cxx_range_for)
target_compile_options( dicom2meshlib PRIVATE -Wall -pedantic -Wextra -Wno-extra-semi )

if(UNIX)
    install(
            TARGETS
            dicom2meshlib
            LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
            ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    )
endif(UNIX)


# Test the library
option(TESTDICOM2MESHLIB "Test library" OFF)
IF(${TESTDICOM2MESHLIB})
    MESSAGE(STATUS "Test library activated")
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    FILE(GLOB_RECURSE  D2M_LIB_TESTS_INC       test/*.h)
    FILE(GLOB_RECURSE  D2M_LIB_TESTS_SRC       test/*.cpp)

    add_executable(runLibTests ${D2M_LIB_TESTS_INC} ${D2M_LIB_TESTS_SRC})
    target_link_libraries(runLibTests ${GTEST_LIBRARIES} dicom2meshlib )
    target_compile_features( runLibTests PRIVATE cxx_range_for )
    target_compile_options( runLibTests PRIVATE -Wall -Wno-extra-semi )
ENDIF()

