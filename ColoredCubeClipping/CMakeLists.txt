PROJECT (ColoredCubeClipping)

ADD_EXECUTABLE( ${PROJECT_NAME} ${EXECUTABLE_FLAG} ColoredCubeClipping.cpp
                vtkColorPolyData.h vtkColorPolyData.cxx
                vtkColoredCubeActor.h vtkColoredCubeActor.cxx )
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${VTK_LIBRARIES} )
