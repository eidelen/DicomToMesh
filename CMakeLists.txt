cmake_minimum_required( VERSION 3.0 )

project( dicom2mesh )

find_package( VTK REQUIRED )
include( ${VTK_USE_FILE} )
set(DICOM_TO_MESH_LIBS ${VTK_LIBRARIES})


# This library can be downloaded at https://github.com/dgobbi/vtk-dicom and
# enables the user to load more types of DICOM images.
option(USE_VTK_DICOM "Use VTK-DICOM Lib (extended DICOM support)" OFF)
if( USE_VTK_DICOM )
    message(STATUS "The external library vtk-dicom, written by David Gobbi, is activated. The project is located under https://github.com/dgobbi/vtk-dicom and relaeased under BSD 3.")
    find_package( DICOM REQUIRED )
    include_directories( ${DICOM_INCLUDE_DIRS} )
    link_directories( ${DICOM_LIBRARY_DIRS} )
    set(DICOM_TO_MESH_LIBS ${DICOM_TO_MESH_LIBS} vtkDICOM)
    add_definitions(-DUSEVTKDICOM)
endif( USE_VTK_DICOM )

include_directories( src )
set(DICOM_TO_MESH_SRC  src/main.cpp src/vtkMeshRoutines.cpp src/vtkDicomRoutines.cpp src/vtkMeshVisualizer.cpp )

add_executable( dicom2mesh ${DICOM_TO_MESH_SRC} )
target_link_libraries( dicom2mesh ${DICOM_TO_MESH_LIBS}  )
target_compile_features( dicom2mesh PRIVATE cxx_range_for )

