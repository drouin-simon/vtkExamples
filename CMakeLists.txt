cmake_minimum_required(VERSION 2.8)

PROJECT (vtkExamples)

IF(NOT VTK_BINARY_DIR)
FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})
ENDIF(NOT VTK_BINARY_DIR)

IF(APPLE)
  SET(EXECUTABLE_FLAG MACOSX_BUNDLE)
ENDIF(APPLE)

add_subdirectory(DomeRendering)
add_subdirectory(ColoredCubeClipping)
