/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>

#include <usModuleRegistry.h>

// Qt
#include <QButtonGroup>
#include <QMessageBox>
#include <QTimer>

// Mitk
#include <mitkDataNode.h>
#include <mitkSurface.h>
#include <mitkNodePredicateProperty.h>
#include <mitkIOUtil.h>
#include <mitkPivotCalibration.h>

// qmitk
#include <QmitkRenderWindow.h>

#include <navAPI.h>

#include "SystemSetupView.h"
#include "SurfaceAdaptation.h"
#include "Markers.h"
#include "ViewCommands.h"

using namespace std;

// Don't forget to initialize the VIEW_ID.
const std::string SystemSetupView::VIEW_ID = "navcas.systemsetup";

SystemSetupView::SystemSetupView()
{
  mNodesManager->InitializeSetup();
}

SystemSetupView::~SystemSetupView()
{
  if (mNodesManager)
  {
    mNodesManager->ShowAllFiducials(false);
  }
}

void SystemSetupView::CreateQtPartControl(QWidget* parent)
{
  // Setting up the UI is a true pleasure when using .ui files, isn't it?
  mControls.setupUi(parent);

  // GUI
  connect(mControls.pbStartCalibration, SIGNAL(clicked()), this, SLOT(OnStartProbeCalibration()));

  // patient tracking
  QButtonGroup* patientTracking = new QButtonGroup(parent);
  patientTracking->addButton(mControls.rbPatientSmall);
  patientTracking->addButton(mControls.rbPatientMedium);
  patientTracking->addButton(mControls.rbPatientBig);
  connect(mControls.rbPatientBig, SIGNAL(toggled(bool)), this, SLOT(OnPatientTrackerChanged()));
  connect(mControls.rbPatientMedium, SIGNAL(toggled(bool)), this, SLOT(OnPatientTrackerChanged()));
  connect(mControls.rbPatientSmall, SIGNAL(toggled(bool)), this, SLOT(OnPatientTrackerChanged()));

  // instrument tracking
  QButtonGroup* instrumentTracking = new QButtonGroup(parent);
  instrumentTracking->addButton(mControls.rbInstrumentSmall);
  instrumentTracking->addButton(mControls.rbInstrumentMedium);
  instrumentTracking->addButton(mControls.rbInstrumentBig);
  connect(mControls.rbInstrumentBig, SIGNAL(toggled(bool)), this, SLOT(OnInstrumentTrackerChanged()));
  connect(mControls.rbInstrumentMedium, SIGNAL(toggled(bool)), this, SLOT(OnInstrumentTrackerChanged()));
  connect(mControls.rbInstrumentSmall, SIGNAL(toggled(bool)), this, SLOT(OnInstrumentTrackerChanged()));

  // navigation mode
  QButtonGroup* navigationMode = new QButtonGroup(parent);
  navigationMode->addButton(mControls.rbTraditional);
  navigationMode->addButton(mControls.rbCustom);
  connect(mControls.rbCustom, SIGNAL(toggled(bool)), this, SLOT(OnNavigationModeChanged()));
  connect(mControls.rbTraditional, SIGNAL(toggled(bool)), this, SLOT(OnNavigationModeChanged()));
  connect(mControls.pbSetInstrument, SIGNAL(clicked()), this, SLOT(OnSetInstrument()));

  mControls.gbInstrument->setVisible(!mControls.rbTraditional->isChecked());

  // warnings
  mControls.lblSmallWarning->setVisible(false);
  mControls.lblMediumWarning->setVisible(false);
  mControls.lblBigWarning->setVisible(false);

  // Navigation
  ConnectNavigationActionsAndFunctions();

  CheckValidRenderWindow();

  LoadPreSettings();
}

void SystemSetupView::LoadPreSettings()
{
  // load navigation mode
  int mode = mNodesManager->GetNavigationMode();
  switch(mode)
  {
  case NodesManager::NavigationMode::Traditional:
    mControls.rbTraditional->setChecked(true);
    break;

  case NodesManager::NavigationMode::InstrumentTracking:
    mControls.rbCustom->setChecked(true);
    break;
  }

  // load reference marker
  int ref = mNodesManager->GetReferenceMarker();
  switch(ref)
  {
  case navAPI::MarkerId::SmallTracker:
    mControls.rbPatientSmall->setChecked(true);
    break;

  case navAPI::MarkerId::MediumTracker:
    mControls.rbPatientMedium->setChecked(true);
    break;

  case navAPI::MarkerId::BigTracker:
    mControls.rbPatientBig->setChecked(true);
    break;
  }

  // load moving marker
  int moving = mNodesManager->GetMovingMarker();
  switch (moving)
  {
  case navAPI::MarkerId::SmallTracker:
    mControls.rbInstrumentSmall->setChecked(true);
    break;

  case navAPI::MarkerId::MediumTracker:
    mControls.rbInstrumentMedium->setChecked(true);
    break;

  case navAPI::MarkerId::BigTracker:
    mControls.rbInstrumentBig->setChecked(true);
    break;
  }
}


void SystemSetupView::SetFocus()
{
  mControls.pbStartCalibration->setFocus();
}

void SystemSetupView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  if ( !GetSite()->GetPage()->IsPartVisible(GetSite()->GetPage()->FindView(GetSite()->GetId())) )
    return;

  mControls.pbSetInstrument->setEnabled(false);

  if (nodes.empty())
    return;

  // Set visible only the selected node
  foreach (mitk::DataNode::Pointer node, nodes)
  {
    // surface is selected
    mitk::Surface* surf = dynamic_cast<mitk::Surface*>(node->GetData());
    if (surf != nullptr)
    {
      mControls.pbSetInstrument->setEnabled(true && mControls.rbCustom->isChecked());
      mSelectedInstrument = node;
      node->SetVisibility(true);

      ViewCommands::Reinit(node,GetDataStorage());
      break;
    }
  }
}


void SystemSetupView::OnSetInstrument()
{
  SetInstrumentSurface(mSelectedInstrument);
}

void SystemSetupView::SetInstrumentSurface(mitk::DataNode::Pointer node)
{
  // search instrument in datastorage and set property to false
  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("navCAS.isInstrument",mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer so = GetDataStorage()->GetSubset(pred);
  for (mitk::DataStorage::SetOfObjects::ConstIterator it=so->Begin(); it != so->End(); ++it)
    it->Value()->SetBoolProperty("navCAS.isInstrument",false);

  // set instrument property to true
  node->SetBoolProperty("navCAS.isInstrument",true);

  // Create low resolution Surface
  SurfaceAdaptation::AttachLowResolutionSurface(GetDataStorage(),node);
  mitk::DataNode::Pointer lowRes = SurfaceAdaptation::GetLowResolutionNode(GetDataStorage(),node);

  // In 2D only show the low resolution surface
  mitk::BaseRenderer* axial = GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetRenderer();
  mitk::BaseRenderer* coronal = GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetRenderer();
  mitk::BaseRenderer* sagittal = GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetRenderer();
  //mitk::BaseRenderer* threeD = GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderer();
  node->SetVisibility(false,axial);
  node->SetVisibility(false,coronal);
  node->SetVisibility(false,sagittal);
  node->SetVisibility(true);
  lowRes->SetVisibility(true,axial);
  lowRes->SetVisibility(true,coronal);
  lowRes->SetVisibility(true,sagittal);
  lowRes->SetVisibility(false);

  mControls.lblCustomSurface->setText(node->GetName().c_str());
  mControls.lblCustomSurface->setStyleSheet("color: rgb(0, 255, 0);");
  cout << "Surface set as instrument" << std::endl;
}


void SystemSetupView::OnValidProbeInView()
{

}

void SystemSetupView::OnInvalidProbeInView()
{

}

void SystemSetupView::OnSingleMarkerInView(navAPI::MarkerId)
{

}

void SystemSetupView::OnMultipleOrNullMarkersInView()
{

}

void SystemSetupView::OnPatientTrackerChanged()
{
  navAPI::MarkerId type = navAPI::MarkerId::None;

  if (mControls.rbPatientMedium->isChecked())
    type = navAPI::MarkerId::MediumTracker;
  else if (mControls.rbPatientBig->isChecked())
    type = navAPI::MarkerId::BigTracker;
  else if (mControls.rbPatientSmall->isChecked())
    type = navAPI::MarkerId::SmallTracker;

  mNodesManager->SetReferenceMarker(type);
  mAPI->SetReferenceMarker(type);

  // Correct instrument tracking marker
  OnNavigationModeChanged();
}

void SystemSetupView::OnInstrumentTrackerChanged()
{
  // uncheck reference marker from selection
  if (mControls.rbInstrumentSmall->isChecked())
  {
    if (mControls.rbPatientSmall->isChecked())
      mControls.rbInstrumentMedium->setChecked(true);
  }
  else if (mControls.rbInstrumentMedium->isChecked())
  {
    if (mControls.rbPatientMedium->isChecked())
      mControls.rbInstrumentSmall->setChecked(true);
  }
  else if (mControls.rbInstrumentBig->isChecked())
  {
    if (mControls.rbPatientBig->isChecked())
      mControls.rbInstrumentMedium->setChecked(true);
  }

  OnNavigationModeChanged();
}

void SystemSetupView::OnMultipleMarkersInView(std::vector<navAPI::MarkerId> list)
{
  int smallCount = std::count(list.begin(),list.end(),navAPI::MarkerId::SmallTracker);
  int mediumCount = std::count(list.begin(),list.end(),navAPI::MarkerId::MediumTracker);
  int bigCount = std::count(list.begin(),list.end(),navAPI::MarkerId::BigTracker);

  mControls.rbPatientSmall->setEnabled(smallCount == 1);
  mControls.rbPatientMedium->setEnabled(mediumCount == 1);
  mControls.rbPatientBig->setEnabled(bigCount == 1);

  bool trackInstrument = !mControls.rbTraditional->isChecked();
  mControls.rbInstrumentSmall->setEnabled(trackInstrument && (smallCount == 1));
  mControls.rbInstrumentMedium->setEnabled(trackInstrument && (mediumCount == 1));
  mControls.rbInstrumentBig->setEnabled(trackInstrument && (bigCount == 1));

  mControls.lblSmallWarning->setVisible(smallCount>1);
  mControls.lblMediumWarning->setVisible(mediumCount>1);
  mControls.lblBigWarning->setVisible(bigCount>1);
}

void SystemSetupView::OnNavigationModeChanged()
{
  // Traditional navigation
  if (mControls.rbTraditional->isChecked())
  {
    // to do: this call could be done inside the navigation mode setting
    mNodesManager->SetUseMovingMarker(false);
    mNodesManager->ShowMovingMarker(false);
    mNodesManager->SetNavigationModeToTraditional();
  }
  // Moving marker
  else
  {
    // set moving marker
    navAPI::MarkerId type = navAPI::MarkerId::None;
    if (mControls.rbInstrumentMedium->isChecked())
      type = navAPI::MarkerId::MediumTracker;
    else if (mControls.rbInstrumentBig->isChecked())
      type = navAPI::MarkerId::BigTracker;
    else if (mControls.rbInstrumentSmall->isChecked())
      type = navAPI::MarkerId::SmallTracker;

    mNodesManager->SetUseMovingMarker(true);
    mNodesManager->SetMovingMarker(type);

    // search for instrument in ds
    auto pred = mitk::NodePredicateProperty::New("navCAS.isInstrument",mitk::BoolProperty::New(true));
    auto so = GetDataStorage()->GetSubset(pred);
    if (so->empty())
    {
      mControls.lblCustomSurface->setText("Select instrument node!");
      mControls.lblCustomSurface->setStyleSheet("color: rgb(255, 0, 0);");
    }
    else
    {
      mControls.lblCustomSurface->setText(so->Begin()->Value()->GetName().c_str());
      mControls.lblCustomSurface->setStyleSheet("color: rgb(0, 255, 0);");

      for (auto it = ++so->Begin(); it != so->End(); ++it)
        it->Value()->SetBoolProperty("navCAS.isInstrument",false);
    }

    mControls.pbSetInstrument->setEnabled(mSelectedInstrument.IsNotNull());
    mNodesManager->SetNavigationModeToInstrumentTracking();
  }

  // enable/disable instrument marker
  mControls.gbInstrument->setVisible(!mControls.rbTraditional->isChecked());
}

void SystemSetupView::UpdateCameraStatusLabels(bool isConnected)
{
  if (isConnected)
  {
    mControls.lblCameraStatus->setText("Camera activated");
    mControls.lblCameraStatus->setStyleSheet("color: rgb(0, 255, 0);");
  }
  else
  {
    mControls.lblCameraStatus->setText("Camera not connected");
    mControls.lblCameraStatus->setStyleSheet("color: rgb(255, 0, 0);");
  }

  mControls.pbStartCalibration->setEnabled(isConnected);
  mNodesManager->ShowAllFiducials(isConnected);
}

bool SystemSetupView::StartNavigation(bool verbose, NavigationType type)
{
  cout << "Trying to detect the navigation camera" << std::endl;
  if (!mAPI->DetectCamera())
  {
    UpdateCameraStatusLabels(false);
    return false;
  }

  cout << "Trying to start navigation" << std::endl;
  NavigationPluginBase::StartNavigation(verbose,type);

  UpdateCameraStatusLabels(IsCameraConnected());

  // allow start zeroing
  return IsCameraConnected();
}


// **************************** //
// ****  Probe calibration **** //
// **************************** //

void SystemSetupView::OnStartProbeCalibration()
{
  if ((mAPI != nullptr) && (mAPI->isRunning()) )
    mAPI->CloseCamera();

  mAPI->StopNavigation();

	// start pivot calibration process
	mControls.lblInformation->setText("Pivot the probe around a known fixed medium marker)");
	mControls.lblInformation->setVisible(true);

  // to do: actually user serie info (temporal probe)
  StartDetectingMarker();
}

void SystemSetupView::OnValidTemporalProbe()
{
  //static int counter =0;
  //cout << "Temporal probe detected: " << ++counter << std::endl;
}

// use for zeroing the probe
void SystemSetupView::StartDetectingMarker()
{
	mTemporalProbePositions.clear();
	mError = 100.0;

  connect(mAPI, SIGNAL(ValidTemporalProbeInView()), this, SLOT(OnValidTemporalProbe()));
  connect(mAPI, SIGNAL(MarkerRelativePosition(unsigned int, vtkMatrix4x4*)), this, SLOT(OnAcquireTemporalPosition(unsigned int, vtkMatrix4x4*)));

	// create calibration database
	if (mControls.cbStoreAcquisitions->isChecked())
	{
		QString tableName = "calibration1";
		mCalibrationDB = Markers::CreateCalibrationTable(tableName);
	}

  // show acquisitions in label
  mControls.lblInformation->setText(QString("0/")+QString::number(mControls.sbAcquisitions->value()));
  mControls.lblInformation->setVisible(true);

  // start calibration navigation
  mAPI->SetSamplingPeriod(20); // 50 Hz
  StartNavigation(true,NavigationPluginBase::NavigationType::Calibration);
}

void SystemSetupView::OnAcquireTemporalPosition(unsigned int marker, vtkMatrix4x4* mat)
{
  const unsigned long ACQUISITIONS = static_cast<unsigned long>(mControls.sbAcquisitions->value());
  static const double MAX_ERROR = 0.1;

  // in case disconnect failed
  if (mTemporalProbePositions.size() == ACQUISITIONS)
    return;

	// store matrix
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  matrix->DeepCopy(mat);
  mTemporalMatrix.push_back(matrix);

	// update probe in the 3d scene
  UpdateRelativeMarker(marker, mat);

	// store probe position in vector for statistics
  //cout << "Acquisition " << mTemporalProbePositions.size() << "/" << ACQUISITIONS << std::endl;
  mControls.lblInformation->setText(QString::number(mTemporalProbePositions.size())+"/"+QString::number(ACQUISITIONS));
	mTemporalProbePositions.push_back(mProbeLastPosition);

	// after N acquisitions compute the error
	if (mTemporalProbePositions.size() == ACQUISITIONS)
	{
		// stop data acquisitions
		disconnect(mAPI, SIGNAL(ValidTemporalProbeInView()), this, SLOT(OnValidTemporalProbe()));
    disconnect(mAPI, SIGNAL(MarkerRelativePosition(unsigned int, vtkMatrix4x4*)), this, SLOT(OnAcquireTemporalPosition(unsigned int, vtkMatrix4x4*)));
		
		// store acquisitions in data base
		if (mControls.cbStoreAcquisitions->isChecked())
		{
      for (unsigned long i=0; i<ACQUISITIONS; i++)
        Markers::StoreCalibrationData(mCalibrationDB, static_cast<int>(i), mTemporalMatrix.at(i), mTemporalProbePositions[i]);
		}

    // build nd from matrix and position
    std::vector<mitk::NavigationData::Pointer> ndVec;
    for (unsigned int a=0; a<mTemporalMatrix.size(); ++a)
    {
      auto nd = mitk::NavigationData::New();

      vnl_matrix_fixed<double,3,3> vnlMatrix;
      for (int i = 0; i < 3; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          vnlMatrix.set(i,j,mTemporalMatrix[a]->GetElement(i,j));
        }
      }

      nd->SetOrientation(mitk::Quaternion(vnlMatrix));
      nd->SetPosition(mTemporalProbePositions[a]);
      nd->SetDataValid(true);
      ndVec.push_back(nd);
    }

    // use MITK pivot calibration
    auto calib = mitk::PivotCalibration::New();
    for (unsigned int i=0; i<ndVec.size(); ++i)
      calib->AddNavigationData(ndVec[i]);

    calib->ComputePivotResult();
    MITK_INFO << "pivot point: " << calib->GetResultPivotPoint();
    MITK_INFO << "rms: " << calib->GetResultRMSError();

    // use the mitk Pivot Calibration to compute the correction
    mitk::Vector3D correction(calib->GetResultPivotPoint().GetDataPointer());
    double errorAfterCorrection = calib->GetResultRMSError();

		// update probe tip position
    if (errorAfterCorrection < mError)
		{
      cout << "Error succesfully lowered from " << mError << " mm to " << errorAfterCorrection << " mm" << std::endl;
			cout << "Correction step: " << correction.GetNorm() << " mm" << std::endl;

      const mitk::Point3D newTip = mNodesManager->UpdateProbeTip(correction);

			cout << "Updated temporal probe position" << std::endl;
			mError = errorAfterCorrection;

      // update tip geomettry file
      Markers::UpdateTemporalProbeTipGeometryFile(Markers::Temporal,124,newTip);

      // to do: does this correctly update the "probe last position"?
		}

		if (mError < MAX_ERROR)
		{
			cout << "Zeroing process converged towards std deviation lower than " << MAX_ERROR << " mm" << std::endl;
			mAPI->CloseCamera();
			return;
		}

    /*
		// continue with calibration?
		QMessageBox msgBox1;
		QString text("Standard deviation (error) is " + QString::number(mError) + " mm");
		msgBox1.setText(text);
		QPushButton *accept1 = msgBox1.addButton(tr("Continue pivot calibration?"), QMessageBox::ActionRole);
		msgBox1.addButton(QMessageBox::Cancel);
		msgBox1.exec();
		if (msgBox1.clickedButton() != accept1)
		{
			mAPI->CloseCamera();
			mCalibrationDB->close();
			return;
		}
    */
    QString error;
    error += QString::number(round(mError*100)/100.0) + " mm";
    mControls.lblError->setText(error);


		// continue
		mTemporalProbePositions.clear();
		mTemporalMatrix.clear();

		connect(mAPI, SIGNAL(ValidTemporalProbeInView()), this, SLOT(OnValidTemporalProbe()));
    connect(mAPI, SIGNAL(MarkerRelativePosition(unsigned int, vtkMatrix4x4*)), this, SLOT(OnAcquireTemporalPosition(unsigned int, vtkMatrix4x4*)));
	}
}


void SystemSetupView::Visible()
{
  cout << "SystemSetup visible: connecting device" << std::endl;

  WaitCursorOn();
  if (!StartNavigation(true))
    QTimer::singleShot(1000,this,SLOT(SilentlyRetryCameraConnection()));
  WaitCursorOff();
}

void SystemSetupView::Hidden()
{
  cout << "SystemSetup hidden: closing device" << std::endl;

  StopNavigation();
}

void SystemSetupView::NodeAdded(const mitk::DataNode* node)
{
  auto pred = mitk::NodePredicateProperty::New("navCAS.systemSetup.isSetupNode",mitk::BoolProperty::New(true));
  if (pred->CheckNode(node))
  {
    cout << "System set up node: " << node->GetName() << std::endl;

    // either remove node or replace the existing node
    QMessageBox msgBox;
    msgBox.setText("Loaded a system set-up configuration.");
    msgBox.setInformativeText("Do you want to override the existing one?");
    auto overrideBtn = msgBox.addButton(tr("Override and use loaded set-up"), QMessageBox::ActionRole);
    msgBox.addButton(tr("Discard loaded set-up"), QMessageBox::ActionRole);
    msgBox.exec();

    auto so = GetDataStorage()->GetSubset(pred);
    // removed old system node
    if (msgBox.clickedButton() == overrideBtn)
    {
      for (auto it = so->Begin(); it!= so->End(); ++it)
      {
        if (it->Value() != node)
          GetDataStorage()->Remove(it->Value());
      }
      LoadPreSettings();
    }
    // remove loaded system node
    else
    {
      GetDataStorage()->Remove(node);
    }
  }
}
