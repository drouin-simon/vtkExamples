#include "vtkCylinderSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProperty.h"
#include "vtkCamera.h"
#include "vtkCallbackCommand.h"
#include "vtkInteractorStyleSwitch.h"
#include "DomeRenderer.h"

vtkRenderWindow * winDome;

void InteractionCallback( vtkObject * caller, unsigned long eid, void * clientdata, void * calldata )
{
    winDome->Render();
}

int main()
{
    // Create cylinder source
    vtkCylinderSource * cylinder = vtkCylinderSource::New();
    cylinder->SetResolution(8);
    cylinder->SetHeight( 12.0 );
    cylinder->SetRadius( 0.1 );

    // Create interacting window cylinder actor
    vtkPolyDataMapper * cylinderMapper = vtkPolyDataMapper::New();
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());
    vtkActor *cylinderActor = vtkActor::New();
    cylinderActor->SetMapper(cylinderMapper);
    cylinderActor->GetProperty()->SetColor(1.0000, 0.3882, 0.2784);

    // Create interacting window
    vtkRenderer * ren = vtkRenderer::New();
    vtkRenderWindow * renWin = vtkRenderWindow::New();
    renWin->AddRenderer( ren );
    vtkRenderWindowInteractor * iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow(renWin);

    vtkInteractorStyleSwitch * is = vtkInteractorStyleSwitch::SafeDownCast( iren->GetInteractorStyle() );
    is->SetCurrentStyleToTrackballCamera();

    ren->AddActor(cylinderActor);
    ren->SetBackground(0.1, 0.2, 0.4);
    renWin->SetSize( 500, 500 );
    renWin->SetPosition( 200, 200 );
    //ren->ResetCamera();
    ren->GetActiveCamera()->SetPosition( 0.0, 0.0, 3.0 );
    ren->GetActiveCamera()->SetFocalPoint( 0.0, 0.0, 0.0 );
    ren->GetActiveCamera()->SetViewUp( 0.0, 1.0, 0.0 );
    ren->GetActiveCamera()->SetViewAngle( 90.0 );
    renWin->Render();

    // Create the Dome cylinder actor
    vtkPolyDataMapper * cylDomeMapper = vtkPolyDataMapper::New();
    cylDomeMapper->SetInputConnection( cylinder->GetOutputPort() );
    vtkActor * cylDomeActor = vtkActor::New();
    cylDomeActor->SetMapper( cylDomeMapper );
    cylDomeActor->GetProperty()->SetColor(1.0000, 0.3882, 0.2784);

    // Create Dome window
    vtkRenderer * renDome = vtkRenderer::New();
    renDome->SetActiveCamera( ren->GetActiveCamera() );
    winDome = vtkRenderWindow::New();
    winDome->AddRenderer( renDome );
    renDome->AddActor( cylDomeActor );
    renDome->SetBackground(0.1, 0.2, 0.4);
    winDome->SetSize( 600, 600 );
    winDome->SetPosition( 700, 200 );

    // Create the delegate that allows us to control rendering
    DomeRenderer * renDomeDelegate = DomeRenderer::New();
    renDomeDelegate->UsedOn();
    renDome->SetDelegate( renDomeDelegate );
    winDome->Render();

    // Setup callback when interaction happens in interaction window
    vtkCallbackCommand * com = vtkCallbackCommand::New();
    com->SetCallback( InteractionCallback );
    renWin->AddObserver( vtkCommand::StartEvent, com );

    // This starts the event loop and as a side effect causes an initial render.
    iren->Start();

    // Exiting from here, we have to delete all the instances that
    // have been created.
    cylinder->Delete();
    cylinderMapper->Delete();
    cylinderActor->Delete();
    ren->Delete();
    renWin->Delete();
    iren->Delete();
    com->Delete();
    cylDomeActor->Delete();
    cylDomeMapper->Delete();
    winDome->Delete();
    renDome->Delete();
    renDomeDelegate->Delete();

    return 0;
}




