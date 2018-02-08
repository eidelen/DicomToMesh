cmake_minimum_required(VERSION 2.8.11)

project(dicom2meshgui)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)

SET( APP_SOURCES main.cpp d2mWidget.cpp )
SET( APP_UIS d2mWidget.ui )
SET( APP_INCLUDE_DIRS . )

include_directories( ${APP_INCLUDE_DIRS} )

QT5_WRAP_UI(APP_FORMS_HEADERS ${APP_UIS} )

add_executable(dicom2meshgui ${APP_SOURCES} ${APP_FORMS_HEADERS} )
target_link_libraries(dicom2meshgui Qt5::Widgets)



