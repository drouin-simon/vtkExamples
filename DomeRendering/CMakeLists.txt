PROJECT (DomeRendering)

ADD_EXECUTABLE( ${PROJECT_NAME} ${EXECUTABLE_FLAG} DomeRendering.cpp DomeRenderer.h DomeRenderer.cpp GlslShader.h GlslShader.cpp vtkOffscreenCamera.h vtkOffscreenCamera.cxx )
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${VTK_LIBRARIES} )
