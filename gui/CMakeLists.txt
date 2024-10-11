cmake_minimum_required(VERSION 3.24)

project(dicom2meshgui)

# Enable necessary CMake features
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 libraries
find_package(Qt6Widgets REQUIRED)

find_package(VTK REQUIRED)
IF("${VTK_MAJOR_VERSION}" LESS 9)
    include(${VTK_USE_FILE})
ENDIF("${VTK_MAJOR_VERSION}" LESS 9)

# Specify sources and UI files
set(APP_SOURCES main.cpp d2mWidget.cpp dicomConverter.cpp rsc/dicom2meshgui.qrc)
set(APP_UIS d2mWidget.ui)
set(APP_INCLUDE_DIRS .)

include_directories(${APP_INCLUDE_DIRS})

qt_wrap_ui(APP_FORMS_HEADERS ${APP_UIS})

add_executable(dicom2meshgui ${APP_SOURCES} ${APP_FORMS_HEADERS})
target_link_libraries(dicom2meshgui dicom2meshlib Qt6::Widgets)

target_compile_features(dicom2meshgui PRIVATE cxx_std_17)
