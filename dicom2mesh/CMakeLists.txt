cmake_minimum_required( VERSION 3.24 )

project( dicom2mesh )

find_package( VTK REQUIRED )

IF("${VTK_MAJOR_VERSION}" LESS 9)
    include( ${VTK_USE_FILE} )
ENDIF("${VTK_MAJOR_VERSION}" LESS 9)

include_directories( inc )
add_executable( dicom2mesh src/main.cpp src/dicom2mesh.cpp )
target_link_libraries( dicom2mesh dicom2meshlib ${VTK_LIBRARIES} )
target_compile_options( dicom2mesh PRIVATE -Wall -Wno-extra-semi )
target_compile_features( dicom2mesh PRIVATE cxx_std_17 )


# Test the library
option(TESTDICOM2MESH "Test Dicom2Mesh" OFF)
IF(${TESTDICOM2MESH})
    MESSAGE(STATUS "Test dicom2mesh activated")

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

    FILE(GLOB_RECURSE  D2M_TESTS_INC       test/*.h)
    FILE(GLOB_RECURSE  D2M_TESTS_SRC       test/*.cpp)

    add_executable( runD2MTests src/dicom2mesh.cpp ${D2M_TESTS_INC} ${D2M_TESTS_SRC} )
    target_link_libraries( runD2MTests dicom2meshlib gtest_main)
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
