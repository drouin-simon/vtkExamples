#include <vtkSmartPointer.h>

// For the rendering pipeline setup:
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>

// For vtkBoxWidget2:
#include <vtkBoxWidget2.h>
#include <vtkBoxRepresentation.h>
#include <vtkCommand.h>
#include <vtkTransform.h>

// For the colored cube
#include "vtkColoredCubeActor.h"
#include "vtkPlanes.h"
#include "vtkPlane.h"

void UpdatePlanes( vtkSmartPointer<vtkColoredCubeActor> cubeActor, vtkSmartPointer<vtkBoxWidget2> boxWidget )
{
    vtkSmartPointer<vtkPlanes> boxPlanes = vtkSmartPointer<vtkPlanes>::New();
    vtkBoxRepresentation * rep = vtkBoxRepresentation::SafeDownCast( boxWidget->GetRepresentation() );
    rep->GetPlanes( boxPlanes );
    double bounds[6];
    for( int i = 0; i < 3; ++i )
    {
        vtkPlane * po = boxPlanes->GetPlane( 2*i );
        bounds[2*i] = po->GetOrigin()[i];
        po = boxPlanes->GetPlane( 2*i + 1 );
        bounds[2*i+1] = po->GetOrigin()[i];
    }

    cubeActor->SetCroppingRegionPlanes( bounds );
}

class vtkBoxCallback : public vtkCommand
{
public:
    static vtkBoxCallback *New()
    {
        return new vtkBoxCallback;
    }

    virtual void Execute( vtkObject *caller, unsigned long, void* )
    {
        vtkSmartPointer<vtkBoxWidget2> boxWidget = vtkBoxWidget2::SafeDownCast(caller);
        UpdatePlanes( CubeActor, boxWidget );
    }

    vtkBoxCallback(){}

    vtkSmartPointer<vtkColoredCubeActor> CubeActor;
};

void KeypressCallback( vtkObject * caller, unsigned long eid, void * clientdata, void * calldata )
{
    vtkRenderWindowInteractor * iren = static_cast<vtkRenderWindowInteractor*>(caller);
    vtkSmartPointer<vtkColoredCubeActor> actor = reinterpret_cast<vtkColoredCubeActor*>(clientdata);
    if( iren->GetKeyCode() == 'c' )
    {
        int back = actor->GetRenderBackface();
        actor->SetRenderBackface( back > 0 ? 0 : 1 );
        iren->Render();
    }
}

double VolumeBounds[6] = { 0.0, 100.0, 0.0, 100.0, 0.0, 100.0 };

int main( int vtkNotUsed( argc ), char* vtkNotUsed( argv )[] )
{
    vtkSmartPointer<vtkColoredCubeActor> cubeActor = vtkSmartPointer<vtkColoredCubeActor>::New();
    cubeActor->SetBounds( VolumeBounds );
    cubeActor->SetCroppingRegionPlanes( VolumeBounds );

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddViewProp( cubeActor );

    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer( renderer );
    renderWindow->SetSize( 1000, 800 );
    renderWindow->SetPosition( 100, 100 );

    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow( renderWindow );

    // Use the "trackball camera" interactor style, rather than the default "joystick camera"
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    style->AutoAdjustCameraClippingRangeOff();
    renderWindowInteractor->SetInteractorStyle( style );

    vtkSmartPointer<vtkBoxWidget2> boxWidget = vtkSmartPointer<vtkBoxWidget2>::New();
    boxWidget->TranslationEnabledOff();
    boxWidget->RotationEnabledOff();
    boxWidget->SetInteractor( renderWindowInteractor );
    boxWidget->GetRepresentation()->SetPlaceFactor( 1 ); // Default is 0.5
    boxWidget->GetRepresentation()->PlaceWidget( VolumeBounds );
    vtkBoxRepresentation * boxRep = vtkBoxRepresentation::SafeDownCast( boxWidget->GetRepresentation() );
    boxRep->InsideOutOn();

    // Set up a callback for the interactor to call so we can manipulate the actor
    vtkSmartPointer<vtkBoxCallback> boxCallback = vtkSmartPointer<vtkBoxCallback>::New();
    boxCallback->CubeActor = cubeActor;
    boxWidget->AddObserver( vtkCommand::InteractionEvent, boxCallback );
    boxWidget->On();

    // Callback for keypress
    vtkSmartPointer<vtkCallbackCommand> keyCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    keyCallback->SetCallback ( KeypressCallback );
    keyCallback->SetClientData( cubeActor );
    renderWindowInteractor->AddObserver( vtkCommand::KeyPressEvent, keyCallback );

    // Once initialized, we can connect the clipping filter
    //UpdatePlanes( mapper, boxWidget );
    renderer->ResetCamera(); // Reposition camera so the whole scene is visible
    renderer->GetActiveCamera()->SetClippingRange( 50, 1000 );

    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}
