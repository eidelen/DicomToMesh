cmake_minimum_required( VERSION 3.0 )

project( dicom2mesh )


add_subdirectory(lib)

find_package( VTK REQUIRED )
include( ${VTK_USE_FILE} )

add_executable( dicom2mesh dicom2mesh/main.cpp )
target_link_libraries( dicom2mesh dicom2meshlib ${VTK_LIBRARIES}  )
target_compile_features( dicom2mesh PRIVATE cxx_range_for )

