#ifndef vtkColoredCubeActor_h
#define vtkColoredCubeActor_h

#include "vtkProp3D.h"

class vtkActor;
class vtkTessellatedBoxSource;
class vtkClipConvexPolyData;
class vtkColorPolyData;
class vtkTriangleFilter;
class vtkPolyDataMapper;
class vtkVolumeProperty;
class vtkPolyData;

class vtkColoredCubeActor : public vtkProp3D
{
public:
  static vtkColoredCubeActor *New();
  vtkTypeMacro(vtkColoredCubeActor,vtkProp3D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Support the standard render methods.
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);

  // Description:
  // Does this prop have some translucent polygonal geometry?
  virtual int HasTranslucentPolygonalGeometry();

  // Description:
  // Shallow copy of an actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Specify the bounding box and cropping region
  vtkGetVector6Macro( Bounds, double );
  vtkSetVector6Macro( Bounds, double );
  vtkGetVector6Macro( CroppingRegionPlanes, double );
  vtkSetVector6Macro( CroppingRegionPlanes, double );

  vtkGetMacro( RenderBackface, int );
  void SetRenderBackface( int r );

  // Description:
  // Get the actors mtime plus consider its properties and texture if set.
  unsigned long int GetMTime();

  // Description:
  // Return the mtime of anything that would cause the rendered image to
  // appear differently. Usually this involves checking the mtime of the
  // prop plus anything else it depends on such as properties, textures
  // etc.
  virtual unsigned long GetRedrawMTime();

protected:
  vtkColoredCubeActor();
  ~vtkColoredCubeActor();

  void DrawCube( vtkPolyData * cubePoly );
  void UpdateProps( vtkViewport * vp );

  // Drawing the bounding box
  int RenderBackface;
  double Bounds[6];
  double CroppingRegionPlanes[6];
  vtkTessellatedBoxSource * BoxSource;
  vtkClipConvexPolyData * BoxClip;
  vtkColorPolyData * BoxColoring;
  vtkTriangleFilter * BoxTriangles;

private:
  vtkColoredCubeActor(const vtkColoredCubeActor&);  // Not implemented.
  void operator=(const vtkColoredCubeActor&);  // Not implemented.
};

#endif

