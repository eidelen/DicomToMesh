cmake_minimum_required( VERSION 3.0 )

project( dicom2mesh )


add_subdirectory(lib)

find_package( VTK REQUIRED )
include( ${VTK_USE_FILE} )

add_executable( dicom2mesh dicom2mesh/main.cpp )
target_link_libraries( dicom2mesh dicom2meshlib ${VTK_LIBRARIES}  )
target_compile_features( dicom2mesh PRIVATE cxx_range_for )


option(BUILD_GUI "Build Dicom2Mesh GUI" OFF)
if(BUILD_GUI)
    add_subdirectory(gui)
endif(BUILD_GUI)


install(
    TARGETS
        dicom2mesh
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin
)
