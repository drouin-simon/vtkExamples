/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkColoredCubeActor.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkColoredCubeActor.h"

#include "vtkTessellatedBoxSource.h"
#include "vtkClipConvexPolyData.h"
#include "vtkPlane.h"
#include "vtkPlaneCollection.h"
#include "vtkColorPolyData.h"
#include "vtkTriangleFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkMatrix4x4.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"  // for vtkStandardNewMacro
#include "vtkNew.h"
#include "vtkUnsignedIntArray.h"

vtkStandardNewMacro(vtkColoredCubeActor);

//----------------------------------------------------------------------------
vtkColoredCubeActor::vtkColoredCubeActor()
{
    RenderBackface = 0;

    this->Bounds[0] = 0.0;
    this->Bounds[1] = 1.0;
    this->Bounds[2] = 0.0;
    this->Bounds[3] = 1.0;
    this->Bounds[4] = 0.0;
    this->Bounds[5] = 1.0;

    this->CroppingRegionPlanes[0]=VTK_DOUBLE_MAX;
    this->CroppingRegionPlanes[1]=VTK_DOUBLE_MIN;
    this->CroppingRegionPlanes[2]=VTK_DOUBLE_MAX;
    this->CroppingRegionPlanes[3]=VTK_DOUBLE_MIN;
    this->CroppingRegionPlanes[4]=VTK_DOUBLE_MAX;
    this->CroppingRegionPlanes[5]=VTK_DOUBLE_MIN;

    BoxSource = vtkTessellatedBoxSource::New();

    BoxClip = vtkClipConvexPolyData::New();
    BoxClip->SetInputConnection( BoxSource->GetOutputPort() );

    // build all clipping planes
    vtkPlaneCollection * allPlanes = vtkPlaneCollection::New();

    vtkPlane * pxMin = vtkPlane::New();
    pxMin->SetNormal( 1.0, 0.0, 0.0 );
    allPlanes->AddItem( pxMin );
    pxMin->Delete();

    vtkPlane * pxMax = vtkPlane::New();
    pxMax->SetNormal( -1.0, 0.0, 0.0 );
    allPlanes->AddItem( pxMax );
    pxMax->Delete();

    vtkPlane * pyMin = vtkPlane::New();
    pyMin->SetNormal( 0.0, 1.0, 0.0 );
    allPlanes->AddItem( pyMin );
    pyMin->Delete();

    vtkPlane * pyMax = vtkPlane::New();
    pyMax->SetNormal( 0.0, -1.0, 0.0 );
    allPlanes->AddItem( pyMax );
    pyMax->Delete();

    vtkPlane * pzMin = vtkPlane::New();
    pzMin->SetNormal( 0.0, 0.0, 1.0 );
    allPlanes->AddItem( pzMin );
    pzMin->Delete();

    vtkPlane * pzMax = vtkPlane::New();
    pzMax->SetNormal( 0.0, 0.0, -1.0 );
    allPlanes->AddItem( pzMax );
    pzMax->Delete();

    vtkPlane * pNear = vtkPlane::New();
    pNear->SetNormal( 1.0, 0.0, 0.0 );
    allPlanes->AddItem( pNear );
    pNear->Delete();

    vtkPlane * pFar = vtkPlane::New();
    pFar->SetNormal( 1.0, 0.0, 0.0 );
    allPlanes->AddItem( pFar );
    pFar->Delete();

    BoxClip->SetPlanes( allPlanes );
    allPlanes->Delete();

    BoxColoring = vtkColorPolyData::New();
    BoxColoring->SetInputConnection( BoxClip->GetOutputPort() );

    BoxTriangles = vtkTriangleFilter::New();
    BoxTriangles->SetInputConnection( BoxColoring->GetOutputPort() );
}

//----------------------------------------------------------------------------
vtkColoredCubeActor::~vtkColoredCubeActor()
{
    BoxSource->Delete();
    BoxClip->Delete();
    BoxColoring->Delete();
    BoxTriangles->Delete();
}

//----------------------------------------------------------------------------
// Shallow copy of an actor.
void vtkColoredCubeActor::ShallowCopy(vtkProp *prop)
{
  vtkColoredCubeActor *a = vtkColoredCubeActor::SafeDownCast(prop);
  if ( a != NULL )
    {
      this->SetBounds( a->GetBounds() );
      this->SetCroppingRegionPlanes( a->GetCroppingRegionPlanes() );
    }

  // Now do superclass
  this->vtkProp3D::ShallowCopy(prop);
}

#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkOpenGL.h"

GLenum vtkDataTypeToGlEnum( int vtkDataType )
{
    GLenum res = GL_FLOAT;
    switch( vtkDataType )
    {
    case VTK_CHAR:
        res = GL_BYTE;
        break;
    case VTK_SIGNED_CHAR:
        res = GL_BYTE;
        break;
    case VTK_UNSIGNED_CHAR:
        res = GL_UNSIGNED_BYTE;
        break;
    case VTK_SHORT:
        res = GL_SHORT;
        break;
    case VTK_UNSIGNED_SHORT:
        res = GL_UNSIGNED_SHORT;
        break;
    case VTK_INT:
        res = GL_INT;
        break;
    case VTK_UNSIGNED_INT:
        res = GL_UNSIGNED_INT;
        break;
    case VTK_LONG:
        res = GL_INT;
        break;
    case VTK_UNSIGNED_LONG:
        res = GL_UNSIGNED_INT;
        break;
    case VTK_FLOAT:
        res = GL_FLOAT;
        break;
    case VTK_DOUBLE:
        res = GL_DOUBLE;
        break;
    }
    return res;
}

void vtkColoredCubeActor::DrawCube( vtkPolyData * cubePoly )
{
    // Vertices
    vtkPoints * points = cubePoly->GetPoints();
    glEnableClientState( GL_VERTEX_ARRAY );
    GLenum vertexType = vtkDataTypeToGlEnum( points->GetDataType() );
    int numberOfPoints = points->GetNumberOfPoints();
    glVertexPointer( 3, vertexType, 0, points->GetData()->GetVoidPointer(0) );

    // Colors
    vtkDataArray * colors = cubePoly->GetPointData()->GetScalars();
    int numberOfColors = colors->GetNumberOfTuples();
    glEnableClientState( GL_COLOR_ARRAY );
    glColorPointer( 3, GL_UNSIGNED_BYTE, 0, colors->GetVoidPointer(0) );

    // Convert triangle indices to unsigned int
    vtkCellArray * cells = cubePoly->GetPolys();
    vtkNew<vtkUnsignedIntArray> triangles;
    triangles->SetNumberOfComponents(3);
    vtkIdType npts;
    vtkIdType *pts;
    while(cells->GetNextCell(npts, pts))
    {
        triangles->InsertNextTuple3(pts[0], pts[1], pts[2]);
    }

    // Draw Triangles
    glDisable( GL_LIGHTING );
    glDrawElements( GL_TRIANGLES, triangles->GetDataSize(), GL_UNSIGNED_INT, triangles->GetVoidPointer(0) );

    // Restore previous state
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
}

//----------------------------------------------------------------------------
int vtkColoredCubeActor::RenderOpaqueGeometry(vtkViewport *vp)
{
  int renderedSomething = 0;
  this->UpdateProps(vp);

  // Make sure polydata is up to date
  BoxTriangles->Update();

  // Draw the cube
  vtkPolyData * cubePoly = vtkPolyData::SafeDownCast( BoxTriangles->GetOutput() );
  glPushAttrib( GL_POLYGON_BIT | GL_ENABLE_BIT );
  glDisable( GL_LIGHTING );
  glEnable( GL_CULL_FACE );
  if( RenderBackface )
      glCullFace( GL_FRONT );
  else
      glCullFace( GL_BACK );
  DrawCube( cubePoly );
  glPopAttrib();

  if( cubePoly->GetNumberOfPoints() > 0 )
      renderedSomething = 1;

  return renderedSomething;
}

//-----------------------------------------------------------------------------
int vtkColoredCubeActor::HasTranslucentPolygonalGeometry()
{
    return 0;
}

//----------------------------------------------------------------------------
void vtkColoredCubeActor::ReleaseGraphicsResources(vtkWindow *win)
{
}

void vtkColoredCubeActor::SetRenderBackface( int r )
{
    this->RenderBackface = r;
    Modified();
}

//----------------------------------------------------------------------------
unsigned long int vtkColoredCubeActor::GetMTime()
{
  unsigned long mTime = this->Superclass::GetMTime();
  return mTime;
}

//----------------------------------------------------------------------------
unsigned long int vtkColoredCubeActor::GetRedrawMTime()
{
  unsigned long mTime = this->GetMTime();
  return mTime;
}

//----------------------------------------------------------------------------
void vtkColoredCubeActor::UpdateProps( vtkViewport *vp )
{
    vtkRenderer * ren = vtkRenderer::SafeDownCast(vp);
    if(!ren)
    {
        vtkErrorMacro(<<"Need a renderer to render this class!");
        return;
    }

    BoxSource->SetBounds(this->Bounds);
    BoxColoring->SetBounds(this->Bounds);

    // Update bounds planes position
    vtkPlaneCollection * allPlanes = BoxClip->GetPlanes();
    allPlanes->GetItem( 0 )->SetOrigin( CroppingRegionPlanes[0], 0.0, 0.0 );
    allPlanes->GetItem( 1 )->SetOrigin( CroppingRegionPlanes[1], 0.0, 0.0 );
    allPlanes->GetItem( 2 )->SetOrigin( 0.0, CroppingRegionPlanes[2], 0.0 );
    allPlanes->GetItem( 3 )->SetOrigin( 0.0, CroppingRegionPlanes[3], 0.0 );
    allPlanes->GetItem( 4 )->SetOrigin( 0.0, 0.0, CroppingRegionPlanes[4] );
    allPlanes->GetItem( 5 )->SetOrigin( 0.0, 0.0, CroppingRegionPlanes[5] );

    // Update near/far planes normal and position
    vtkCamera * cam = ren->GetActiveCamera();
    double near = cam->GetClippingRange()[0];
    double far = cam->GetClippingRange()[1];

    // Get the inverse of the volume matrix
    vtkMatrix4x4 * invVolMatrix = vtkMatrix4x4::New();
    invVolMatrix->DeepCopy( this->GetMatrix() );
    invVolMatrix->Invert();

    // Transform camera position and target to volume space
    double pos[4] = { 0.0, 0.0, 0.0, 1.0 };
    cam->GetPosition( pos );
    invVolMatrix->MultiplyPoint( pos, pos );

    double target[4] = { 0.0, 0.0, 0.0, 1.0 };
    cam->GetFocalPoint( target );
    invVolMatrix->MultiplyPoint( target, target );

    // Compute direction of projection
    double dir[3] = { 1.0, 0.0, 0.0 };
    vtkMath::Subtract( target, pos, dir );
    vtkMath::Normalize( dir );

    // Compute an offset for the near and far planes to avoid being clipped
    // due to floating-point precision
    // offset calculation stolen from vtkOpenGLVolumeRaycastMapper : choose arbitrary
    // offset. if the offset is larger than the distance between near and far point,
    // it will not work, in this case we pick a fraction of the near-far distance.
    double distNearFar = far - near;
    double offset = 0.001; // some arbitrary small value.
    if( offset >= distNearFar)
      offset = distNearFar / 1000.0;
    near += offset;
    far -= offset;

    // Compute near plane
    double nearOrigin[3] = { 0.0, 0.0, 0.0 };
    double nearNormal[3] = { 1.0, 0.0, 0.0 };
    for( int i = 0; i < 3; ++i )
    {
        nearOrigin[i] = pos[i] + dir[i] * near;
        nearNormal[i] = dir[i];
    }
    allPlanes->GetItem(6)->SetOrigin( nearOrigin );
    allPlanes->GetItem(6)->SetNormal( nearNormal );

    // Compute far plane
    double farOrigin[3] = { 0.0, 0.0, 0.0 };
    double farNormal[3] = { 1.0, 0.0, 0.0 };
    for( int i = 0; i < 3; ++i )
    {
        farOrigin[i] = pos[i] + dir[i] * far;
        farNormal[i] = -dir[i];
    }
    allPlanes->GetItem(7)->SetOrigin( farOrigin );
    allPlanes->GetItem(7)->SetNormal( farNormal );

    BoxClip->Modified();
}

//----------------------------------------------------------------------------
void vtkColoredCubeActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Bounds: ( ";
  for( int i = 0; i < 5; ++i )
      os << this->Bounds[i] << ", ";
  os << this->Bounds[5] << " )" << endl;
}
