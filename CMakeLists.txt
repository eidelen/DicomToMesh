cmake_minimum_required( VERSION 3.0 )

project( dicom2mesh )

find_package( VTK REQUIRED )
include( ${VTK_USE_FILE} )
SET(DICOM_TO_MESH_LIBS ${VTK_LIBRARIES})

option(USE_VTK_DICOM "Use VTK-DICOM Lib (extended DICOM support)" OFF)
if( USE_VTK_DICOM )
    MESSAGE(STATUS "The external library vtk-dicom, written by David Gobbi, is activated. The project is located under https://github.com/dgobbi/vtk-dicom and relaeased under BSD 3.")
    find_package( DICOM REQUIRED )
    include_directories( ${DICOM_INCLUDE_DIRS} )
    link_directories( ${DICOM_LIBRARY_DIRS} )
    SET(DICOM_TO_MESH_LIBS ${DICOM_TO_MESH_LIBS} vtkDICOM)
    ADD_DEFINITIONS(-DUSEVTKDICOM)
endif( USE_VTK_DICOM )

add_executable( dicom2mesh main.cpp vtkMeshRoutines.h vtkMeshRoutines.cpp vtkDicomRoutines.h vtkDicomRoutines.cpp )
target_link_libraries( dicom2mesh ${DICOM_TO_MESH_LIBS}  )
target_compile_features( dicom2mesh PRIVATE cxx_range_for )

