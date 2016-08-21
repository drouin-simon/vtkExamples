CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT( mf-gui )

#==================================================================
# Look for VTK (required)
#==================================================================
find_package(VTK 6.2 COMPONENTS vtkRenderingOpenGL vtkRenderingImage
    vtkInteractionStyle vtkInteractionWidgets vtkGUISupportQt NO_MODULE )
include(${VTK_USE_FILE})

#==================================================================
# Look for OpenCV (required)
#==================================================================
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

#==================================================================
# Look for Qt (required)
#==================================================================
find_package( Qt5 COMPONENTS Widgets REQUIRED )
include_directories( ${Qt5Widgets_INCLUDE_DIRS} )
set(CMAKE_AUTOMOC TRUE)

# include generated files
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB_RECURSE CODE_FILES *.cpp)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})

add_executable( ${CMAKE_PROJECT_NAME} MACOSX_BUNDLE ${UI_HEADERS} ${CODE_FILES} )
target_link_libraries( ${CMAKE_PROJECT_NAME} Qt5::Widgets ${VTK_LIBRARIES} ${OpenCV_LIBS} )
