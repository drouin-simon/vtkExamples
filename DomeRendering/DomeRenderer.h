#ifndef __DomeRenderer_h_
#define __DomeRenderer_h_

#include "vtkRendererDelegate.h"

class vtkCamera;
class GlslShader;

class DomeRenderer : public vtkRendererDelegate
{

public:

    static DomeRenderer * New() { return new DomeRenderer; }
    DomeRenderer();
    virtual void Render( vtkRenderer * r );
    int GetCubeTextureSize() { return m_cubeTextureSize; }

protected:

    void DrawFishEye( int w, int h );
    void DrawCubeMap();
    bool LoadGLExtensions( vtkRenderer * r );
    bool SetupFrameBuffer();

    vtkCamera * m_renderCam;
    GlslShader * m_domeShader;

    bool m_isInit;
    int m_cubeTextureSize;
    unsigned m_cubeTextureId;
    unsigned m_fbId;
    unsigned m_depthRenderBufferId;
};

#endif
