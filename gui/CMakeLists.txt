cmake_minimum_required(VERSION 3.22 )

project(dicom2meshgui)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)

find_package(Qt5Widgets REQUIRED)
find_package(VTK REQUIRED)

IF("${VTK_MAJOR_VERSION}" LESS 9)
    include( ${VTK_USE_FILE} )
ENDIF("${VTK_MAJOR_VERSION}" LESS 9)

SET( APP_SOURCES main.cpp d2mWidget.cpp dicomConverter.cpp rsc/dicom2meshgui.qrc)
SET( APP_UIS d2mWidget.ui )
SET( APP_INCLUDE_DIRS . )

include_directories( ${APP_INCLUDE_DIRS} )

QT5_WRAP_UI(APP_FORMS_HEADERS ${APP_UIS} )

add_executable(dicom2meshgui ${APP_SOURCES} ${APP_FORMS_HEADERS} )
target_link_libraries(dicom2meshgui dicom2meshlib Qt5::Widgets)
target_compile_features(dicom2meshgui PRIVATE cxx_std_17 )
