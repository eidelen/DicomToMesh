cmake_minimum_required( VERSION 3.0 )

project( dicom2mesh )

find_package( VTK REQUIRED )
include( ${VTK_USE_FILE} )

include_directories( inc )
add_executable( dicom2mesh src/main.cpp src/dicom2mesh.cpp )
target_link_libraries( dicom2mesh dicom2meshlib ${VTK_LIBRARIES}  )
target_compile_features( dicom2mesh PRIVATE cxx_range_for )

if(UNIX)
    install(
            TARGETS
            dicom2mesh
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )
endif(UNIX)