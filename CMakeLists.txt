cmake_minimum_required( VERSION 3.0 )

project( dicom2mesh )

find_package( VTK REQUIRED )
include( ${VTK_USE_FILE} )

add_executable( dicom2mesh main.cpp vtkMeshRoutines.h vtkMeshRoutines.cpp )
target_link_libraries( dicom2mesh ${VTK_LIBRARIES}  )
target_compile_features( dicom2mesh PRIVATE cxx_range_for )
