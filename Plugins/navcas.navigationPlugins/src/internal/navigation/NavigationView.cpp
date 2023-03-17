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
#include <QInputDialog>
#include <QtSql>
#include <QFileDialog>
#include <QStandardPaths>

// Vtk
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellLocator.h>

// Mitk
#include <mitkDataNode.h>
#include <mitkSurface.h>
#include <mitkImage.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLayoutAnnotationRenderer.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>

// qmitk
#include <QmitkRenderWindow.h>

#include <QFileDialog>

#include <navAPI.h>
#include "NavigationView.h"
#include "IOCommands.h"
#include "LabeledPointSetMapper3D.h"
#include "LabeledPointSetMapper2D.h"
#include "RegistrationPointsMetrics.h"

using namespace std;

// Don't forget to initialize the VIEW_ID.
const std::string NavigationView::VIEW_ID = "navcas.navigation";

NavigationView::NavigationView() :
  mSelectedNode(nullptr),
  mMeasurementPointsNode(nullptr)
{
  SetMoveCrosshair(true);

  std::string rendererID = string("stdmulti.widget3");
  for (unsigned int i=0; i<2; i++)
  {
    mNavigationErrorText.push_back(mitk::TextAnnotation2D::New());

    mNavigationErrorText[i]->SetFontSize(14);
    mNavigationErrorText[i]->SetText("");
    mNavigationErrorText[i]->SetOpacity(1);
    // The position of the Annotation can be set to a fixed coordinate on the display.
    mitk::Point2D pos;
    pos[0] = 10;
    pos[1] = 15*(i+1);
    mNavigationErrorText[i]->SetPosition2D(pos);

    // The LayoutAnnotationRenderer can place the TextAnnotation2D at some defined corner positions
    mitk::LayoutAnnotationRenderer::AddAnnotation(mNavigationErrorText[i], rendererID, mitk::LayoutAnnotationRenderer::TopLeft, 5, 5, 1);
  }

  // direction of probe test
  double normal[3] = {0,0,1};
  double center[3] = {0.0};
  mArrowTolerance = 5.0;
  mArrowTest = new ArrowSource(normal,center,mArrowTolerance,0.5);
  auto surf = mitk::Surface::New();
  surf->SetVtkPolyData(mArrowTest->GetOutput());
}

NavigationView::~NavigationView()
{
  StopNavigation();

  delete mArrowTest;
}

void NavigationView::CreateQtPartControl(QWidget* parent)
{
  // Setting up the UI is a true pleasure when using .ui files, isn't it?
  mControls.setupUi(parent);

  // Angle and Offset error
  connect(mNodesManager, SIGNAL(AngleError(double)), this, SLOT(OnInformAngleError(double)));
  connect(mNodesManager, SIGNAL(OffsetError(double)), this, SLOT(OnInformOffsetError(double)));
  connect(mNodesManager, SIGNAL(Stimulation(mitk::Point3D,mitk::Vector3D)), this, SLOT(OnStimulationMoved(mitk::Point3D,mitk::Vector3D)));

  connect(mControls.pbStoreRelativeMatrix, SIGNAL(clicked()), this, SLOT(OnStoreRelativeMatrix()));

  // measurements
  connect(mControls.pbStartMeasurements, SIGNAL(clicked()), this, SLOT(OnStartMeasurements()));
  connect(mControls.pbExportMeasurements, SIGNAL(clicked()), this, SLOT(OnExportMeasurements()));

	// Navigation
	ConnectNavigationActionsAndFunctions();

  CheckValidRenderWindow();
  CheckValidRegistration();
}


void NavigationView::CheckValidRegistration()
{
  if (IsReadyToNavigate())
  {
    std::cout << "Registration transformation: " << std::endl;

    for (int i = 0; i < 4; ++i)
    {
      for (int j = 0; j< 4; ++j)
        cout << mRegistrationTransformation->GetElement(i,j) << ", ";
      cout << std::endl;
    }
  }
}

void NavigationView::SetFocus()
{
}

void NavigationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  // check registration
  CheckValidRegistration();

  mControls.pbStartMeasurements->setEnabled(false);
  mSelectedNode = nullptr;

  for (auto node : nodes)
  {
    auto ps = dynamic_cast<mitk::PointSet*>(node->GetData());
    if (ps == nullptr)
      continue;

    mControls.pbStartMeasurements->setEnabled(true);
    mSelectedNode = node;
    return;
  }
}

bool NavigationView::StartNavigation(bool verbose, NavigationType)
{
  cout << "Checking for valid registration" << std::endl;

  if (!IsReadyToNavigate())
  {
    if (verbose)
      NodesManager::ErrorMessage("No valid registration confirmed","Please go back to Registration");

    return false;
  }

  // Load system setup configuration
  navAPI::MarkerId referenceMarker = mNodesManager->GetReferenceMarker();
  if (referenceMarker == navAPI::MarkerId::None)
  {
    if (verbose)
      NodesManager::ErrorMessage("Invalid marker for reference image","Please configure system setup parameters");

    mControls.lblInfo->setText("No reference marker was assigned in System Setup");
    mControls.lblInfo->setStyleSheet("QLabel { color : red; }");
    return false;
  }
  mControls.lblInfo->setText("");

  cout << "Trying to start navigation" << std::endl;

  // check navigation mode
  bool isInstrumentMode = (mNodesManager->GetNavigationMode() == NodesManager::NavigationMode::InstrumentTracking);
  if (isInstrumentMode)
  {
    // check moving marker
    navAPI::MarkerId instrumenteMarker = mNodesManager->GetMovingMarker();
    if (instrumenteMarker == navAPI::MarkerId::None)
    {
      if (verbose)
        NodesManager::ErrorMessage("Invalid marker for instrument","Please configure system setup parameters");

      mControls.lblInfo->setText("No instrument marker was assigned in System Setup");
      mControls.lblInfo->setStyleSheet("QLabel { color : red; }");
      return false;
    }
    else
    {
      mNodesManager->SetMovingMarker(instrumenteMarker);
    }
  }

  return NavigationPluginBase::StartNavigation(verbose);
}

void NavigationView::OnInformAngleError(double angle)
{
  stringstream text;
  text << "Angle error: " << angle << "°";
  mNavigationErrorText[0]->SetText(text.str().c_str());

  if (abs(angle) < 3.0)
    mNavigationErrorText[0]->SetColor(0, 1, 0);
  else if (abs(angle) < 8.0)
    mNavigationErrorText[0]->SetColor(1, 1, 0);
  else
    mNavigationErrorText[0]->SetColor(1, 0, 0);
}

void NavigationView::OnInformOffsetError(double offset)
{
  stringstream text;
  text << "Offset error: " << offset << " mm";
  mNavigationErrorText[1]->SetText(text.str().c_str());

  if (abs(offset) < 2.0)
    mNavigationErrorText[1]->SetColor(0, 1, 0);
  else if (abs(offset) < 5.0)
    mNavigationErrorText[1]->SetColor(1, 1, 0);
  else
    mNavigationErrorText[1]->SetColor(1, 0, 0);
}

void NavigationView::OnValidProbeInView()
{
  // enable point measurements insertion
  if (!mProbeEnabled)
  {
    mProbeEnabled = true;
    for (int i = 0; i< mMeasurementPointGroupStack.size(); i++)
    {
      mMeasurementPointGroupStack[i]->pbSet->setEnabled(true);
    }
  }
}

void NavigationView::OnInvalidProbeInView()
{
  // disable point measurements insertion
  if (mProbeEnabled)
  {
    mProbeEnabled = false;
    for (int i = 0; i< mMeasurementPointGroupStack.size(); i++)
    {
      mMeasurementPointGroupStack[i]->pbSet->setEnabled(false);
    }
  }
}

void NavigationView::OnMultipleMarkersInView(std::vector<navAPI::MarkerId> markers)
{
  unsigned int count=0;
  for (navAPI::MarkerId id : markers)
  {
    if ((id == navAPI::MarkerId::SmallTracker) || (id == navAPI::MarkerId::MediumTracker) || (id == navAPI::MarkerId::BigTracker))
      ++count;
  }
  mControls.pbStoreRelativeMatrix->setEnabled(count >= 2);
}

void NavigationView::OnSingleMarkerInView(navAPI::MarkerId)
{

}

void NavigationView::OnMultipleOrNullMarkersInView()
{

}

void NavigationView::Visible()
{
  cout << "NavigationView visible: connecting device" << std::endl;

  //HideNonPlanningNodes();

  WaitCursorOn();
  if (!StartNavigation(true))
    QTimer::singleShot(1000,this,SLOT(SilentlyRetryCameraConnection()));
  WaitCursorOff();
}

void NavigationView::Hidden()
{
  cout << "NavigationView hidden: closing device" << std::endl;
  StopNavigation();

  // open TMS viewer

}


void NavigationView::HideNonPlanningNodes()
{
  auto planningPred = mitk::NodePredicateProperty::New("navCAS.planning.useNode",mitk::BoolProperty::New(true));
  auto showPred = mitk::NodePredicateNot::New(planningPred);

  // hide every other node apart from the planning ones and the probe or instrument/coil
  auto so = GetDataStorage()->GetSubset(showPred);
  for (auto it = so->Begin(); it != so->End(); ++it)
  {
    it->Value()->SetVisibility(false);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  // open viewer perspective
}

void NavigationView::OnStartSimulation()
{
  // conigure navigation
  //mAPI->SetReferenceMarker(navAPI::MarkerId::SmallTracker);
  mNodesManager->SetReferenceMarker(navAPI::MarkerId::SmallTracker);
  mNodesManager->SetMovingMarker(navAPI::MarkerId::MediumTracker);
  mNodesManager->SetNavigationModeToInstrumentTracking();

  // reference marker
  auto refMat = vtkSmartPointer<vtkMatrix4x4>::New();
  refMat->Identity();
  mNodesManager->UpdateRelativeMarker(0,refMat);

  // moving marker
  mNodesManager->SetUseMovingMarker(true);
  auto movMat = vtkSmartPointer<vtkMatrix4x4>::New();
  movMat->Identity();
  mNodesManager->UpdateRelativeMarker(1,movMat);

  // probe
  auto probeMat = vtkSmartPointer<vtkMatrix4x4>::New();
  probeMat->Identity();
  mNodesManager->UpdateRelativeMarker(2,probeMat);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void NavigationView::OnStartMeasurements()
{
  // measurements are contrasted against the original planning (in image coordinates)

  // load planning points from datastorage (pointset)
  mMeasurementPointsNode = mSelectedNode;
  mMeasurementPlanningPointset = dynamic_cast<mitk::PointSet*>(mMeasurementPointsNode->GetData());

  LabeledPointSetMapper3D::Pointer mapper = LabeledPointSetMapper3D::New();
  mMeasurementPointsNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, mapper);

  LabeledPointSetMapper2D::Pointer mapper2D = LabeledPointSetMapper2D::New();
  mMeasurementPointsNode->SetStringProperty("default label","M");
  mMeasurementPointsNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard2D, mapper2D);

  // fill point group stack
  mMeasurementPointGroupStack = PointGroupStack(mMeasurementPlanningPointset);

  // add widgets and build connections
  for (int i=0; i<mMeasurementPointGroupStack.size(); i++)
  {
    auto group = mMeasurementPointGroupStack[i];
    mControls.vlPoints->addWidget(group->window);
    group->pbShow->setVisible(false);
    group->pbSet->setEnabled(false);
    group->pbRemove->setEnabled(false);

    connect(group->pbSet, SIGNAL(clicked()), this, SLOT(OnSetMeasurementPoint()));
    connect(group->pbRemove, SIGNAL(clicked()), this, SLOT(OnRemoveMeasurementPoint()));
  }

  // create real pointset
  mMeasurementRealPointset = mitk::PointSet::New();
  mMeasurementRealPointsNode = mitk::DataNode::New();
  mMeasurementRealPointsNode->SetData(mMeasurementRealPointset);
  LabeledPointSetMapper3D::Pointer realMapper = LabeledPointSetMapper3D::New();
  mMeasurementRealPointsNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, realMapper);
  mMeasurementRealPointsNode->SetColor(0,0,1);
  mMeasurementRealPointsNode->SetName("Real measured points");
  GetDataStorage()->Add(mMeasurementRealPointsNode);

  LabeledPointSetMapper2D::Pointer realMapper2D = LabeledPointSetMapper2D::New();
  mMeasurementRealPointsNode->SetStringProperty("default label","M");
  mMeasurementRealPointsNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard2D, realMapper2D);

  // allow both to set a particular point or to autodetect point by position

  mControls.pbExportMeasurements->setEnabled(true);
}


void NavigationView::OnSetMeasurementPoint()
{
  int pointNumber = mMeasurementPointGroupStack.findPointGroupFromSender(this->sender(),PointGroupStack::SetPoint);

  cout << "Set point " << pointNumber << std::endl;

  int pointId = pointNumber-1;
  if (pointId < 0)
    return;

  mPointId = pointId;

  AverageProbeAcquisitions(10);
}

void NavigationView::NewAveragedAcquisition(const mitk::Point3D point, double sd, int /*rep*/)
{
  int pointId = mPointId;
  mMeasurementRealPointset->InsertPoint(pointId,point);
  mMeasurementPointGroupStack[pointId]->pbRemove->setEnabled(true);

  // compute local error
  double error = (mMeasurementRealPointset->GetPoint(pointId) - mMeasurementPlanningPointset->GetPoint(pointId)).GetNorm();
  mMeasurementPointGroupStack[pointId]->SetError(error);
  mMeasurementPointGroupStack[pointId]->SetStdDev(sd);

  QString style = error < 1.0 ? "color:green" : "color:red";

  // paint already-measured points in green
  mMeasurementPointGroupStack[pointId]->window->setStyleSheet(style);

  WaitCursorOff();
}


void NavigationView::OnRemoveMeasurementPoint()
{
  int pointNumber = mMeasurementPointGroupStack.findPointGroupFromSender(this->sender(),PointGroupStack::RemovePoint);

  cout << "Remove point " << pointNumber << std::endl;

  int pointId = pointNumber-1;
  if (pointId < 0)
    return;

  mMeasurementPointGroupStack[pointId]->pbRemove->setEnabled(false);
  mMeasurementRealPointset->RemovePointIfExists(pointId);

  mMeasurementPointGroupStack[pointId]->window->setStyleSheet("");
}

void NavigationView::OnExportMeasurements()
{
  // get planning pointset
  if (mMeasurementPlanningPointset.IsNull() || (mMeasurementPlanningPointset->GetSize() == 0))
  {
    NodesManager::ErrorMessage("Cannot export measurements","The planned pointset is empty!");
    return;
  }

  // get real pointset
  if (mMeasurementRealPointset.IsNull() || (mMeasurementRealPointset->GetSize() == 0))
  {
    NodesManager::ErrorMessage("Cannot export measurements","The measured pointset is empty!");
    return;
  }

  // get last path
  QString lastFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

  // always add stored patient name
  lastFilePath+= "/" + IOCommands::GetCurrentPatientName() + ".csv";

  // export to csv
  QString filename = QFileDialog::getSaveFileName(0, "Export measurements", lastFilePath,
                                                  "CSV files (.csv)", 0, 0);

  IOCommands::ExportMeasurementsToCSV(filename,mMeasurementPlanningPointset,mMeasurementRealPointset);
}

void NavigationView::OnStoreRelativeMatrix()
{
  auto matrix = mNodesManager->GetLastMovingMarkerRelativeMatrix();

  QSettings settings("CAS", "navCAS");
  QString lastPath = settings.value("Relative position between markers after registration", "/home/").toString();

  QString filename = QFileDialog::getSaveFileName(0,
    tr("Save database..."),
    lastPath,
    tr("Text file (*.txt)"));

  if (filename.isEmpty())
    return;

  IOCommands::StoreRegistrationInDisk(matrix,IOCommands::SeriesType::None,filename.toStdString());

  settings.setValue("Relative position between markers after registration",filename);
}




