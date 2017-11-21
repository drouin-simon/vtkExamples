#include "DomeRenderer.h"
#include "vtkRenderer.h"
#include "vtkgl.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkOffscreenCamera.h"
#include "vtkMath.h"
#include "GlslShader.h"

DomeRenderer::DomeRenderer()
{
    m_isInit = false;
    m_cubeTextureSize = 2000;
    m_cubeTextureId = 0;
    m_fbId = 0;
    m_depthRenderBufferId = 0;
    m_domeShader = 0;

    m_renderCam = vtkOffscreenCamera::New();
    m_renderCam->SetViewAngle( 90.0 );   
}

bool DomeRenderer::LoadGLExtensions( vtkRenderer * r )
{
    vtkOpenGLRenderWindow * win = vtkOpenGLRenderWindow::SafeDownCast( r->GetRenderWindow() );
    if( !win )
    {
        vtkErrorMacro(<< "DomeRenderer: No Valid OpenGL render window." );
        return false;
    }

    vtkOpenGLExtensionManager * man = win->GetExtensionManager();

    bool canLoad = true;
    if( !man->ExtensionSupported("GL_VERSION_2_0") )
    {
        canLoad = false;
        vtkErrorMacro( << "DomeRenderer: OpenGL 2.0 required but not supported" );
    }

    if( !man->ExtensionSupported("GL_EXT_framebuffer_object" ) )
    {
        canLoad = false;
        vtkErrorMacro( << "DomeRenderer: GL_EXT_framebuffer_object is required but not supported" );
    }

    if( !man->ExtensionSupported("GL_ARB_texture_cube_map") )
    {
        vtkErrorMacro( << "DomeRenderer: GL_ARB_texture_cube_map is required but not supported" );
    }

    if( !man->ExtensionSupported("GL_ARB_seamless_cube_map") )
    {
        vtkErrorMacro( << "DomeRenderer: GL_ARB_seamless_cube_map is required but not supported" );
    }

    // Really load now that we know everything is supported
    if( canLoad )
    {
        man->LoadExtension( "GL_VERSION_1_2" );
        man->LoadExtension( "GL_VERSION_1_3" );
        man->LoadExtension( "GL_VERSION_2_0" );
        man->LoadExtension( "GL_EXT_framebuffer_object" );
        man->LoadExtension( "GL_ARB_texture_cube_map" );
        man->LoadExtension( "GL_ARB_seamless_cube_map" );
    }

    return canLoad;
}

bool DomeRenderer::SetupFrameBuffer()
{
    // backup prev FB binding
    GLint prevFrameBuffer;
    glGetIntegerv( vtkgl::FRAMEBUFFER_BINDING_EXT, &prevFrameBuffer );

    // Create and bind frame buffer object
    vtkgl::GenFramebuffersEXT( 1, &m_fbId );
    vtkgl::BindFramebufferEXT( vtkgl::FRAMEBUFFER_EXT, m_fbId );

    // Create and initialize cube texture
    glGenTextures( 1, &m_cubeTextureId );
    glBindTexture( GL_TEXTURE_CUBE_MAP, m_cubeTextureId );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+0, 0, GL_RGBA8, m_cubeTextureSize, m_cubeTextureSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+1, 0, GL_RGBA8, m_cubeTextureSize, m_cubeTextureSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+2, 0, GL_RGBA8, m_cubeTextureSize, m_cubeTextureSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+3, 0, GL_RGBA8, m_cubeTextureSize, m_cubeTextureSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+4, 0, GL_RGBA8, m_cubeTextureSize, m_cubeTextureSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+5, 0, GL_RGBA8, m_cubeTextureSize, m_cubeTextureSize, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

    // Attach one of the faces of the Cubemap texture to this FBO
    vtkgl::FramebufferTexture2DEXT( vtkgl::FRAMEBUFFER_EXT, vtkgl::COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_cubeTextureId, 0 );

    // Create and bind depth buffer used for all faces of the cube and attach it to FBO
    vtkgl::GenRenderbuffersEXT( 1, &m_depthRenderBufferId );
    vtkgl::BindRenderbufferEXT( vtkgl::RENDERBUFFER_EXT, m_depthRenderBufferId );
    vtkgl::RenderbufferStorageEXT( vtkgl::RENDERBUFFER_EXT, vtkgl::DEPTH_COMPONENT24, m_cubeTextureSize, m_cubeTextureSize );
    vtkgl::BindRenderbufferEXT( vtkgl::RENDERBUFFER_EXT, 0 );

    // Attach depth render buffer to the FBO
    vtkgl::FramebufferRenderbufferEXT( vtkgl::FRAMEBUFFER_EXT, vtkgl::DEPTH_ATTACHMENT_EXT, vtkgl::RENDERBUFFER_EXT, m_depthRenderBufferId );

    // Check if FBO is complete and config supported by system
    GLenum ret = vtkgl::CheckFramebufferStatusEXT( vtkgl::FRAMEBUFFER_EXT );
    if( ret != vtkgl::FRAMEBUFFER_COMPLETE_EXT )
    {
        vtkgl::BindFramebufferEXT( vtkgl::FRAMEBUFFER_EXT, prevFrameBuffer );
        glDeleteTextures( 1, &m_cubeTextureId );
        m_cubeTextureId = 0;
        vtkgl::DeleteFramebuffersEXT( 1, &m_fbId );
        m_fbId = 0;
        vtkgl::DeleteRenderbuffersEXT( 1, &m_depthRenderBufferId );
        m_depthRenderBufferId = 0;
        return false;
    }

    vtkgl::BindFramebufferEXT( vtkgl::FRAMEBUFFER_EXT, prevFrameBuffer );

    return true;
}

void DomeRenderer::Render( vtkRenderer * r )
{
    if( !m_isInit )
    {
        m_isInit = LoadGLExtensions( r );
        if( !m_isInit )
            return;
    }

    if( m_fbId == 0 )
    {
        SetupFrameBuffer();
        if( m_fbId == 0 )
            return;
    }

    // Make sure we don't recursively call ourself
    this->UsedOff();

    // Let vtk think the size of the window is size of the cube texture while we render
    int winSize[2];
    winSize[0] = r->GetRenderWindow()->GetSize()[0];
    winSize[1] = r->GetRenderWindow()->GetSize()[1];

    // Remember if light was following cam
    int lightFollows = r->GetLightFollowCamera();

    // Clear the screen
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Get camera params
    vtkCamera * cam = r->GetActiveCamera();
    cam->OrthogonalizeViewUp();
    double * pos = cam->GetPosition();
    double * focalPt = cam->GetFocalPoint();
    double * up = cam->GetViewUp();
    double dir[3];
    dir[0] = focalPt[0] - pos[0];
    dir[1] = focalPt[1] - pos[1];
    dir[2] = focalPt[2] - pos[2];
    double dist = vtkMath::Normalize( dir );

    r->SetActiveCamera( m_renderCam );
    r->LightFollowCameraOn();
    
    // Start drawing to the FBO
    GLint prevFrameBuffer;
    glGetIntegerv( vtkgl::FRAMEBUFFER_BINDING_EXT, &prevFrameBuffer );
    vtkgl::BindFramebufferEXT( vtkgl::FRAMEBUFFER_EXT, m_fbId );

    //-------------------------------------
    // Front (Z+)
    //-------------------------------------
    vtkgl::FramebufferTexture2DEXT( vtkgl::FRAMEBUFFER_EXT, vtkgl::COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, m_cubeTextureId, 0 );
    m_renderCam->SetPosition( pos );
    m_renderCam->SetFocalPoint( focalPt );
    m_renderCam->SetViewUp( up );
    r->Render();

    //-------------------------------------
    // Up  (Y-)
    //-------------------------------------
    r->LightFollowCameraOff();
    vtkgl::FramebufferTexture2DEXT( vtkgl::FRAMEBUFFER_EXT, vtkgl::COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, m_cubeTextureId, 0 );
    double fpUp[3];
    fpUp[0] = pos[0] + dist * up[0];
    fpUp[1] = pos[1] + dist * up[1];
    fpUp[2] = pos[2] + dist * up[2];
    m_renderCam->SetFocalPoint( fpUp );
    m_renderCam->SetViewUp( -dir[0], -dir[1], -dir[2] );
    r->Render();

    //-------------------------------------
    // Down (Y+)
    //-------------------------------------
    vtkgl::FramebufferTexture2DEXT( vtkgl::FRAMEBUFFER_EXT, vtkgl::COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, m_cubeTextureId, 0 );
    double fpDown[3];
    fpDown[0] = pos[0] - dist * up[0];
    fpDown[1] = pos[1] - dist * up[1];
    fpDown[2] = pos[2] - dist * up[2];
    m_renderCam->SetFocalPoint( fpDown );
    m_renderCam->SetViewUp( dir[0], dir[1], dir[2] );
    r->Render();

    //-------------------------------------
    // Right (X+)
    //-------------------------------------
    vtkgl::FramebufferTexture2DEXT( vtkgl::FRAMEBUFFER_EXT, vtkgl::COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_cubeTextureId, 0 );
    double dirRight[3];
    vtkMath::Cross( dir, up, dirRight );
    double fpRight[3];
    fpRight[0] = pos[0] + dist * dirRight[0];
    fpRight[1] = pos[1] + dist * dirRight[1];
    fpRight[2] = pos[2] + dist * dirRight[2];
    m_renderCam->SetFocalPoint( fpRight );
    m_renderCam->SetViewUp( up );
    r->Render();

    //-------------------------------------
    // Left (X-)
    //-------------------------------------
    vtkgl::FramebufferTexture2DEXT( vtkgl::FRAMEBUFFER_EXT, vtkgl::COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, m_cubeTextureId, 0 );
    double fpLeft[3];
    fpLeft[0] = pos[0] - dist * dirRight[0];
    fpLeft[1] = pos[1] - dist * dirRight[1];
    fpLeft[2] = pos[2] - dist * dirRight[2];
    m_renderCam->SetFocalPoint( fpLeft );
    m_renderCam->SetViewUp( up );
    r->Render();

    //-------------------------------------
    // Dome render
    //-------------------------------------
    vtkgl::BindFramebufferEXT( vtkgl::FRAMEBUFFER_EXT, prevFrameBuffer );

    //DrawCubeMap();
    DrawFishEye( winSize[0], winSize[1] );

    // Revert to prev state
    r->SetActiveCamera( cam );
    r->SetLightFollowCamera( lightFollows );
    this->UsedOn();
}

static const char * domePixelShaderCode = " \
uniform samplerCube cubemap; \
\
void main() \
{ \
    vec2 d = gl_TexCoord[0].xy; \
    float r = length( d ); \
    if( r > 1.0 ) \
        discard; \
    vec2 dunit = normalize( d ); \
    float phi = 1.57079632679 * r; \
    vec3 cubeTexCoord = vec3( 1.0, 1.0, 1.0 ); \
    cubeTexCoord.xy = dunit * sin( phi ); \
    cubeTexCoord.z = cos( phi ); \
    gl_FragColor = textureCube( cubemap, cubeTexCoord ); \
}";

void DomeRenderer::DrawFishEye( int w, int h )
{
    glPushAttrib( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );

    glViewport( 0, 0, w, h );

    // Push projection
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0, w, 0, h, -1, 1 );

    // Push modelview
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    glColor4d( 1.0, 1.0, 1.0, 1.0 );
    glEnable( vtkgl::TEXTURE_CUBE_MAP_ARB );
    glBindTexture( vtkgl::TEXTURE_CUBE_MAP_ARB, m_cubeTextureId );

    // Setup shader
    if( !m_domeShader )
    {
        m_domeShader = new GlslShader;
        m_domeShader->AddShaderMemSource( domePixelShaderCode );
        if( !m_domeShader->Init() )
        {
            m_domeShader->Delete();
            vtkErrorMacro( << "DomeRenderer: can't initialize GLSL shader." );
            return;
        }
    }

    m_domeShader->UseProgram( true );
    m_domeShader->SetVariable( "cubemap", int(0) );

    // Draw a square
    double squareSize = (double)std::min( w, h);
    double xMin = ((double)w - squareSize) * 0.5;
    double xMax = (double)w - xMin;
    double yMin = ((double)h - squareSize) * 0.5;
    double yMax = (double)h - yMin;
    glBegin( GL_QUADS );
    {
        glTexCoord2d( -1.0,  1.0 );  glVertex2d( xMin, yMin );
        glTexCoord2d(  1.0,  1.0 );  glVertex2d( xMax, yMin );
        glTexCoord2d(  1.0, -1.0 );	 glVertex2d( xMax, yMax );
        glTexCoord2d( -1.0, -1.0 );  glVertex2d( xMin, yMax );
    }
    glEnd();

    m_domeShader->UseProgram( false );

    glBindTexture( vtkgl::TEXTURE_CUBE_MAP_ARB, 0 );
    glDisable( vtkgl::TEXTURE_CUBE_MAP_ARB );

    // Pop modelview
    glPopMatrix();

    // Pop projection
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    // back to modelview
    glMatrixMode( GL_MODELVIEW );

    glPopAttrib();
}

void DomeRenderer::DrawCubeMap()
{
    glPushAttrib( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );

    // Push projection
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0, 600, 0, 600, -1, 1 );

    // Push modelview
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    glColor4d( 1.0, 1.0, 1.0, 1.0 );
    glEnable( vtkgl::TEXTURE_CUBE_MAP_ARB );
    glBindTexture( vtkgl::TEXTURE_CUBE_MAP_ARB, m_cubeTextureId );

    // Front (Z+)
    double x = 200.0;
    double y = 200.0;
    glBegin( GL_QUADS );
    {
        glTexCoord3d( -1.0,  1.0, 1.0 );    glVertex2d( x, y );
        glTexCoord3d(  1.0,  1.0, 1.0 );    glVertex2d( x + 200.0, y );
        glTexCoord3d(  1.0, -1.0, 1.0 );	glVertex2d( x + 200.0, y + 200.0 );
        glTexCoord3d( -1.0, -1.0, 1.0 );    glVertex2d( x, y + 200.0 );
    }
    glEnd();

    // Up (Y-)
    x = 200.0;
    y = 400.0;
    glBegin( GL_QUADS );
    {
        glTexCoord3d( -1.0, -1.0,  1.0 );    glVertex2d( x, y );
        glTexCoord3d(  1.0, -1.0,  1.0 );    glVertex2d( x + 200.0, y );
        glTexCoord3d(  1.0, -1.0, -1.0 );	 glVertex2d( x + 200.0, y + 200.0 );
        glTexCoord3d( -1.0, -1.0, -1.0 );    glVertex2d( x, y + 200.0 );
    }
    glEnd();

    // Down (Y+)
    x = 200.0;
    y = 0.0;
    glBegin( GL_QUADS );
    {
        glTexCoord3d( -1.0,  1.0, -1.0 );    glVertex2d( x, y );
        glTexCoord3d(  1.0,  1.0, -1.0 );    glVertex2d( x + 200.0, y );
        glTexCoord3d(  1.0,  1.0,  1.0 );	 glVertex2d( x + 200.0, y + 200.0 );
        glTexCoord3d( -1.0,  1.0,  1.0 );    glVertex2d( x, y + 200.0 );
    }
    glEnd();

    // Right (X+)
    x = 400.0;
    y = 200.0;
    glBegin( GL_QUADS );
    {
        glTexCoord3d(  1.0,  1.0,  1.0 );    glVertex2d( x, y );
        glTexCoord3d(  1.0,  1.0, -1.0 );    glVertex2d( x + 200.0, y );
        glTexCoord3d(  1.0, -1.0, -1.0 );	 glVertex2d( x + 200.0, y + 200.0 );
        glTexCoord3d(  1.0, -1.0,  1.0 );    glVertex2d( x, y + 200.0 );
    }
    glEnd();

    // Left (X-)
    x = 0.0;
    y = 200.0;
    glBegin( GL_QUADS );
    {
        glTexCoord3d( -1.0,  1.0, -1.0 );    glVertex2d( x, y );
        glTexCoord3d( -1.0,  1.0,  1.0 );    glVertex2d( x + 200.0, y );
        glTexCoord3d( -1.0, -1.0,  1.0 );	  glVertex2d( x + 200.0, y + 200.0 );
        glTexCoord3d( -1.0, -1.0, -1.0 );    glVertex2d( x, y + 200.0 );
    }
    glEnd();

    glBindTexture( vtkgl::TEXTURE_CUBE_MAP_ARB, 0 );
    glDisable( vtkgl::TEXTURE_CUBE_MAP_ARB );

    // Pop modelview
    glPopMatrix();

    // Pop projection
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    // back to modelview
    glMatrixMode( GL_MODELVIEW );

    glPopAttrib();
}
