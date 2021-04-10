cmake_minimum_required( VERSION 3.0 )

project( dicom2mesh )

find_package( VTK REQUIRED )
include( ${VTK_USE_FILE} )

include_directories( inc )
add_executable( dicom2mesh src/main.cpp src/dicom2mesh.cpp )
target_link_libraries( dicom2mesh dicom2meshlib ${VTK_LIBRARIES} )
target_compile_options( dicom2mesh PRIVATE -Wall -pedantic -Wextra -Wno-extra-semi )
target_compile_features( dicom2mesh PRIVATE cxx_std_17 )


# Test the library
option(TESTDICOM2MESH "Test Dicom2Mesh" OFF)
IF(${TESTDICOM2MESH})
    MESSAGE(STATUS "Test dicom2mesh activated")
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    FILE(GLOB_RECURSE  D2M_TESTS_INC       test/*.h)
    FILE(GLOB_RECURSE  D2M_TESTS_SRC       test/*.cpp)

    add_executable( runD2MTests src/dicom2mesh.cpp ${D2M_TESTS_INC} ${D2M_TESTS_SRC} )
    target_link_libraries( runD2MTests ${GTEST_LIBRARIES} dicom2meshlib )
    target_compile_features( runD2MTests PRIVATE cxx_std_17 )
    target_compile_options( runD2MTests PRIVATE -Wall -Wno-extra-semi )
ENDIF()


if(UNIX)
    install(
            TARGETS
            dicom2mesh
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )
endif(UNIX)