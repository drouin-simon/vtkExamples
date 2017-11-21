/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOffscreenCamera.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOffscreenCamera - OpenGL camera for offscreen rendering
// .SECTION Description
// vtkOffscreenCamera is the same as vtkOpenGLCamera, except it doesn't rely
// on the render window to obtain the image size. It unfortunately drops support
// for tile rendering and picking in the process.

#ifndef __vtkOffscreenCamera_h
#define __vtkOffscreenCamera_h

#include "vtkCamera.h"

class vtkOffscreenCamera : public vtkCamera
{
public:
  static vtkOffscreenCamera *New();
  vtkTypeMacro(vtkOffscreenCamera,vtkCamera);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement base class method.
  void Render(vtkRenderer *ren);

  void UpdateViewport(vtkRenderer *ren);
  
protected:  
  vtkOffscreenCamera() {};
  ~vtkOffscreenCamera() {};
private:
  vtkOffscreenCamera(const vtkOffscreenCamera&);  // Not implemented.
  void operator=(const vtkOffscreenCamera&);  // Not implemented.
};

#endif
