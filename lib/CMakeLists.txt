﻿cmake_minimum_required( VERSION 3.24 )

project( dicom2meshlib )

find_package( VTK REQUIRED )

IF("${VTK_MAJOR_VERSION}" LESS 9)
    include( ${VTK_USE_FILE} )
ENDIF("${VTK_MAJOR_VERSION}" LESS 9)

set(DICOM_TO_MESH_LIB_LIBS ${VTK_LIBRARIES})

FILE(GLOB  DICOM_TO_MESH_LIB_INC       inc/*.h)
FILE(GLOB  DICOM_TO_MESH_LIB_SRC       src/*.cpp)
set(DICOM_TO_MESH_INC_DIRS inc)

# This library can be downloaded at https://github.com/dgobbi/vtk-dicom and
# enables the user to load more types of DICOM images.
option(USE_VTK_DICOM "Use VTK-DICOM Lib (extended DICOM support)" OFF)
IF("${VTK_MAJOR_VERSION}" LESS 9)
    if( USE_VTK_DICOM )
        message(STATUS "The external library vtk-dicom, written by David Gobbi, is activated. The project is located under https://github.com/dgobbi/vtk-dicom and relaeased under BSD 3.")
        find_package( DICOM REQUIRED )
        include_directories( ${DICOM_INCLUDE_DIRS} )
        link_directories( ${DICOM_LIBRARY_DIRS} )
        set(DICOM_TO_MESH_LIB_LIBS ${DICOM_TO_MESH_LIB_LIBS} vtkDICOM)
        add_definitions(-DUSEVTKDICOM)
        set(DICOM_TO_MESH_LIB_INC ${DICOM_TO_MESH_LIB_INC} inc/vtkdicom/dicomRoutinesExtended.h)
        set(DICOM_TO_MESH_LIB_SRC ${DICOM_TO_MESH_LIB_SRC} src/vtkdicom/dicomRoutinesExtended.cpp)
        set(DICOM_TO_MESH_INC_DIRS ${DICOM_TO_MESH_INC_DIRS} inc/vtkdicom)
    endif( USE_VTK_DICOM )
ELSE("${VTK_MAJOR_VERSION}" LESS 9)
    if( USE_VTK_DICOM )
        message(STATUS "vtk-dicom is a module in VTK 9. Build VTK 9 with enabled vtk-dicom.")
        add_definitions(-DUSEVTKDICOM)
        set(DICOM_TO_MESH_LIB_INC ${DICOM_TO_MESH_LIB_INC} inc/vtkdicom/dicomRoutinesExtended.h)
        set(DICOM_TO_MESH_LIB_SRC ${DICOM_TO_MESH_LIB_SRC} src/vtkdicom/dicomRoutinesExtended.cpp)
        set(DICOM_TO_MESH_INC_DIRS ${DICOM_TO_MESH_INC_DIRS} inc/vtkdicom)
    endif( USE_VTK_DICOM )
ENDIF("${VTK_MAJOR_VERSION}" LESS 9)

include_directories( ${DICOM_TO_MESH_INC_DIRS} )

ADD_LIBRARY(dicom2meshlib STATIC ${DICOM_TO_MESH_LIB_INC} ${DICOM_TO_MESH_LIB_SRC})
target_link_libraries(dicom2meshlib ${DICOM_TO_MESH_LIB_LIBS})
target_include_directories(dicom2meshlib INTERFACE ${DICOM_TO_MESH_INC_DIRS})
target_compile_features(dicom2meshlib PRIVATE cxx_std_17)
target_compile_options( dicom2meshlib PRIVATE -Wall -Wno-extra-semi )

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

    # Download and build gtest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    FILE(GLOB_RECURSE  D2M_LIB_TESTS_INC       test/*.h)
    FILE(GLOB_RECURSE  D2M_LIB_TESTS_SRC       test/*.cpp)

    add_executable(runLibTests ${D2M_LIB_TESTS_INC} ${D2M_LIB_TESTS_SRC})
    target_link_libraries(runLibTests dicom2meshlib gtest_main)
    target_compile_features( runLibTests PRIVATE cxx_std_17 )
    target_compile_options( runLibTests PRIVATE -Wall -Wno-extra-semi )
ENDIF()

