/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>

#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <usModuleRegistry.h>

// Qt
#include <QMessageBox>

// Vtk
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkPolyData.h>
#include <vtkCellLocator.h>

// Mitk
#include <mitkDataNode.h>
#include <mitkSurface.h>
#include <mitkImage.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLayoutAnnotationRenderer.h>

// qmitk
#include <QmitkRenderWindow.h>

#include <navAPI.h>
#include "NavigationPluginBase.h"
#include "ViewCommands.h"

using namespace std;

NavigationPluginBase::NavigationPluginBase():
  mMoveCrosshair(false),
  mCameraConnected(false),
  mCancelCameraConection(false),
  mProbeRepetitions(10)
{
  mNodesManager = new NodesManager(GetDataStorage());
  mAPI = new navAPI;

  mNodesManager->LoadMarkersGeometries();
  mNodesManager->CreateMarkers();
  mNodesManager->ShowAllFiducials(false);
  mNodesManager->ShowAllProbes(false);

  mRegistrationTransformation = vtkSmartPointer<vtkMatrix4x4>::New();
}

NavigationPluginBase::~NavigationPluginBase()
{
  cout << "Plugin base destructor called" << std::endl;

  StopNavigation();

  delete mAPI;

  if (mNodesManager != nullptr)
    delete mNodesManager;
}

void NavigationPluginBase::ConnectNavigationActionsAndFunctions()
{
  connect(mAPI, SIGNAL(MultipleMarkersInView(std::vector<navAPI::MarkerId>)), this, SLOT(OnMultipleMarkersInView(std::vector<navAPI::MarkerId>)));
  connect(mAPI, SIGNAL(SingleMarkerInView(navAPI::MarkerId)), this, SLOT(OnSingleMarkerInView(navAPI::MarkerId)));
	connect(mAPI, SIGNAL(MultipleOrNullMarkersInView()), this, SLOT(OnMultipleOrNullMarkersInView()));
	connect(mAPI, SIGNAL(MarkerRelativePosition(unsigned int, vtkMatrix4x4*)), this, SLOT(UpdateRelativeMarker(unsigned int, vtkMatrix4x4*)));
	connect(mAPI, SIGNAL(ValidProbeInView()), this, SLOT(OnValidProbeInView()));
	connect(mAPI, SIGNAL(InvalidProbeInView()), this, SLOT(OnInvalidProbeInView()));
}

bool NavigationPluginBase::IsReadyToNavigate()
{
  // Registration transform is loaded from valid patient
  // bug: should differentiate registration from navigation
  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("navCAS.registration.isReferencePatientImage",mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer so = GetDataStorage()->GetSubset(pred);

  if (so->size() != 1)
  {
    cout << "Found " << so->size() << " current registration images" << std::endl;
    return false;
  }

  bool isRegistered = false;
  so->Begin()->Value()->GetBoolProperty("navCAS.registration.isRegistered",isRegistered);

  if (!isRegistered)
    return false;

  mRegistrationTransformation = mNodesManager->GetTransformFromNode(so->Begin()->Value());

  if (mRegistrationTransformation == nullptr)
    return false;

  return true;
}

void NavigationPluginBase::CheckValidRenderWindow()
{
  QmitkRenderWindow* axial = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("axial");
  cout << "Checking if render window is valid: " << axial->isValid() << std::endl;

  if (!axial->isValid())
  {
    QTimer::singleShot(500,this,SLOT(CheckValidRenderWindow()));
    return;
  }

  // Configure render windows for data interactor
  QmitkRenderWindow* coronal = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("coronal");
  QmitkRenderWindow* sagittal = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("sagittal");

  mNodesManager->SetRenderWindow(axial,coronal,sagittal);

  connect(axial, SIGNAL(destroyed(QObject*)), this, SLOT(RenderWindowClosed()));

  Hide3DCrosshair();
}

void NavigationPluginBase::Hide3DCrosshair()
{
  // Hide 3d crosshair
  QmitkRenderWindow* threeD = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("3d");
  ViewCommands::Set3DCrosshairVisibility(false,GetDataStorage(),threeD);
}

void NavigationPluginBase::RenderWindowClosed()
{
  std::cout << "Render window destroyed" << std::endl;

  mNodesManager->SetRenderWindow(nullptr,nullptr,nullptr);
}



bool NavigationPluginBase::StartNavigation(bool verbose, NavigationType type)
{
  //setlocale(LC_ALL,"C");
	std::cout << "Checking decimal precision: " << std::atof("1.2345") << std::endl;
  mCameraConnected=false;

  if (!mAPI->InitializeCamera())
  {
    if (verbose)
    {
      QMessageBox msgBox;
      msgBox.setText("No camera detected!");
      msgBox.setInformativeText("Please make sure the navigation camera is correctly plugged in.");
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
    }
    return false;
  }

  if (type == NavigationType::Traditional)
  {
    // Hide instrument node
    // to do: get instrument from NodesManager
    mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("navCAS.isInstrument",mitk::BoolProperty::New(true));
    mitk::DataStorage::SetOfObjects::ConstPointer so = GetDataStorage()->GetSubset(pred);
    if (so->size() == 1)
      so->Begin()->Value()->SetVisibility(false);
  }
	
  // Hide moving surface in 2D if application is TMS (for dissociated navigation should not do this)
  auto pred = mitk::NodePredicateProperty::New("navCAS.isMovingMarkerSurface",mitk::BoolProperty::New(true));
  auto so = GetDataStorage()->GetSubset(pred);
  if (so->size() == 1)
  {
    mitk::BaseRenderer* axial = GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetRenderer();
    mitk::BaseRenderer* coronal = GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetRenderer();
    mitk::BaseRenderer* sagittal = GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetRenderer();
    so->Begin()->Value()->SetVisibility(false,axial);
    so->Begin()->Value()->SetVisibility(false,coronal);
    so->Begin()->Value()->SetVisibility(false,sagittal);
  }

  // load geometries according to navigation type
  if ((Traditional == type) || (RegistrationInstrument == type) || (RegistrationPatient == type))
  {
    // Add geometries and start navigation
    mAPI->AddGeometry(navAPI::Probe);
    mAPI->AddGeometry(navAPI::SmallTracker);
    mAPI->AddGeometry(navAPI::MediumTracker);
    mAPI->AddGeometry(navAPI::BigTracker);
    //mAPI->AddGeometry(navAPI::LineProbe);
  }
  else if (Calibration == type)
  {
    mAPI->AddGeometry(navAPI::MediumTracker);
    mAPI->AddGeometry(navAPI::BigTracker);
    mAPI->AddGeometry(navAPI::TemporalProbe);
    //mAPI->AddGeometry(navAPI::TemporalLineProbe);
  }

  mAPI->start();

  // Update nodesManager members according to systemSetup node in datastorage
  mNodesManager->UpdateNavigationConfiguration();

  // error: I'm setting the wrong reference marker while registrating the instrument!
  if (type == RegistrationInstrument)
    mAPI->SetReferenceMarker(mNodesManager->GetMovingMarker());
  else
    mAPI->SetReferenceMarker(mNodesManager->GetReferenceMarker());

  mCameraConnected=true;

  // disable 2D views interaction
  GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetVtkRenderWindow()->GetInteractor()->Disable();
  GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetVtkRenderWindow()->GetInteractor()->Disable();
  GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetVtkRenderWindow()->GetInteractor()->Disable();

  return true;
}

void NavigationPluginBase::StopNavigation()
{
  mAPI->StopNavigation();
  WaitCursorOff();

	if (mAPI->CloseCamera())
		cout << "Camera closed succesfully" << std::endl;

  if (mNodesManager != nullptr)
  {
    mNodesManager->HideProbe2D();
    mNodesManager->ShowAllProbes(false);
    mNodesManager->ShowAllFiducials(false);

    mNodesManager->ShowMovingMarker(false);
  }

  // avoid possible re-conection intention
  mCancelCameraConection=true;

  // restore views interaction
  GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetVtkRenderWindow()->GetInteractor()->Enable();
  GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetVtkRenderWindow()->GetInteractor()->Enable();
  GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetVtkRenderWindow()->GetInteractor()->Enable();
}

void NavigationPluginBase::UpdateRelativeMarker(unsigned int m, vtkMatrix4x4* matrix)
{
  // Transform position using registration matrix
  vtkSmartPointer<vtkMatrix4x4> registeredMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkMatrix4x4::Multiply4x4(mRegistrationTransformation,matrix,registeredMatrix);

  mNodesManager->UpdateRelativeMarker(m,registeredMatrix);

  // probe
  if (m == 2)
  {
    // to do: does this apply to the temporal probe?
    mProbeLastPosition = mNodesManager->GetProbeLastPosition();
    mProbeDirection = mNodesManager->GetProbeDirection();
    //cout << "Probe position: " << mProbeLastPosition << std::endl;

    // move crosshair
		if (mMoveCrosshair)
			GetRenderWindowPart()->SetSelectedPosition(mProbeLastPosition);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void NavigationPluginBase::SilentlyRetryCameraConnection()
{
  if ( !GetSite()->GetPage()->IsPartVisible(GetSite()->GetPage()->FindView(GetSite()->GetId())) )
    return;

  if (!mCameraConnected)
  {
    //try to silently connect camera
    if (!StartNavigation(false))
      QTimer::singleShot(1000,this,SLOT(SilentlyRetryCameraConnection()));
    else
      WaitCursorOff();
  }
  else
    WaitCursorOff();
}

void NavigationPluginBase::AverageProbeAcquisitions(int repetitions)
{
  WaitCursorOn();
  mProbeRepetitions = repetitions;

  mCurrentAcquisitionPoint.Fill(0.0);
  connect(mAPI, SIGNAL(ValidProbeInView()), this, SLOT(OnAddAcquisition()));
}

void NavigationPluginBase::OnAddAcquisition()
{
  static int repetitions = 0;
  mCurrentAcquisitionPoint += mitk::Vector3D(mProbeLastPosition.GetDataPointer());
  mAcquisitionPoints.push_back(mProbeLastPosition);
  if (++repetitions == mProbeRepetitions)
  {
    disconnect(mAPI, SIGNAL(ValidProbeInView()), this, SLOT(OnAddAcquisition()));

    // average point acquisitions
    for (int i=0; i<3; i++)
      mCurrentAcquisitionPoint[i] /= mProbeRepetitions;

    WaitCursorOff();
    repetitions = 0;// is this neccessary?

    // compute std deviation of acquisitions
    double sum = 0.0;
    for (unsigned int i=0; i<mAcquisitionPoints.size(); i++)
    {
      double dist2 = vtkMath::Distance2BetweenPoints(mAcquisitionPoints[i].GetDataPointer(),
                                                     mCurrentAcquisitionPoint.GetDataPointer());
      sum += dist2;
    }
    mCurrentAcquisitionPointSd = std::sqrt(sum / mAcquisitionPoints.size());

    NewAveragedAcquisition(mCurrentAcquisitionPoint,mCurrentAcquisitionPointSd,mProbeRepetitions);
    //SetMeasurementPoint(mPointId,mCurrentAcquisitionPoint,mCurrentAcquisitionPointSd);
    mAcquisitionPoints.clear();
  }
}



void NavigationPluginBase::OnValidProbeInView()
{

}

void NavigationPluginBase::OnInvalidProbeInView()
{

}
