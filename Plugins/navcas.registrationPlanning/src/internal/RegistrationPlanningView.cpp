/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>

#include <QHBoxLayout>

#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <usModuleRegistry.h>

// test
#include <vtkPSphereSource.h>

// Mitk
#include <mitkDataNode.h>
#include <mitkSurface.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkImage.h>
#include <mitkCameraController.h>

// us
#include <usGetModuleContext.h>

#include "RegistrationPlanningView.h"
#include "PlanningInteractor.h"

// qmitk
#include <QmitkRenderWindow.h>

#include "ViewCommands.h"
#include "NodesManager.h"
#include "PointGroupStack.h"

using namespace std;

// Don't forget to initialize the VIEW_ID.
const std::string RegistrationPlanningView::VIEW_ID = "navcas.views.RegistrationPlanningView";

RegistrationPlanningView::RegistrationPlanningView()
{
  // Create pointset for primary registration points
  mNodesManager = new NodesManager(GetDataStorage());
  mRegistrationPointsNode = mNodesManager->CreatePlannedPoints();
  mRegistrationPoints = dynamic_cast<mitk::PointSet*>(mRegistrationPointsNode->GetData());

  // Start simple interaction
  us::Module* module = us::GetModuleContext()->GetModule("Interactors");
  mInteractor = PlanningInteractor::New(GetDataStorage());
  mInteractor->LoadStateMachine("PlanningInteraction.xml", module);
  mInteractor->SetEventConfig("PlanningInteractionConfig.xml", module);
  mitk::DataNode::Pointer interactorNode = mitk::DataNode::New();
  mitk::GeometryData::Pointer gd = mitk::GeometryData::New();
  interactorNode->SetData(gd);
  interactorNode->SetBoolProperty("helper object",!NodesManager::GetShowHelperObjects());
  interactorNode->SetName("Registration Planning Interactor");
  interactorNode->SetVisibility(true);
  GetDataStorage()->Add(interactorNode);
  mInteractor->SetDataNode(interactorNode);

  // test
  auto sphere = vtkSmartPointer<vtkSphereSource>::New();
  sphere->Update();
  auto node = mitk::DataNode::New();
  node->SetName("test sphere");
  auto surf = mitk::Surface::New();
  surf->SetVtkPolyData(sphere->GetOutput());
  double spacing[3] = {0.25,0.25,0.25};
  surf->GetGeometry()->SetSpacing(mitk::Vector3D(spacing));
  node->SetData(surf);
  GetDataStorage()->Add(node);
}

RegistrationPlanningView::~RegistrationPlanningView()
{
  mitk::DataNode::Pointer node = mInteractor->GetDataNode();
  if (GetDataStorage()->Exists(node))
    GetDataStorage()->Remove(node);

  mInteractor->SetDataNode(nullptr);

  // hide registration points
  mRegistrationPointsNode->SetVisibility(false);

  delete mNodesManager;
}

void RegistrationPlanningView::CreateQtPartControl(QWidget* parent)
{
  // Setting up the UI is a true pleasure when using .ui files, isn't it?
  mControls.setupUi(parent);

  connect(mControls.pbSetP1, SIGNAL(clicked()), this, SLOT(OnSetPoint()));
  connect(mControls.pbSetP2, SIGNAL(clicked()), this, SLOT(OnSetPoint()));
  connect(mControls.pbSetP3, SIGNAL(clicked()), this, SLOT(OnSetPoint()));
  connect(mControls.pbSetP4, SIGNAL(clicked()), this, SLOT(OnSetPoint()));
  connect(mControls.pbSetP5, SIGNAL(clicked()), this, SLOT(OnSetPoint()));

  connect(mControls.pbRemoveP1, SIGNAL(clicked()), this, SLOT(OnRemovePoint()));
  connect(mControls.pbRemoveP2, SIGNAL(clicked()), this, SLOT(OnRemovePoint()));
  connect(mControls.pbRemoveP3, SIGNAL(clicked()), this, SLOT(OnRemovePoint()));
  connect(mControls.pbRemoveP4, SIGNAL(clicked()), this, SLOT(OnRemovePoint()));
  connect(mControls.pbRemoveP5, SIGNAL(clicked()), this, SLOT(OnRemovePoint()));

  connect(mControls.pbShowP1, SIGNAL(clicked()), this, SLOT(OnShowPoint()));
  connect(mControls.pbShowP2, SIGNAL(clicked()), this, SLOT(OnShowPoint()));
  connect(mControls.pbShowP3, SIGNAL(clicked()), this, SLOT(OnShowPoint()));
  connect(mControls.pbShowP4, SIGNAL(clicked()), this, SLOT(OnShowPoint()));
  connect(mControls.pbShowP5, SIGNAL(clicked()), this, SLOT(OnShowPoint()));

  connect(mControls.pbAddPoint, SIGNAL(clicked()), this, SLOT(OnAddPoint()));

  connect(mControls.pbStoreCoilPoints, SIGNAL(clicked()), this, SLOT(OnStoreCoilRegistrationPoints()));
  connect(mControls.pbLoadCoilPoints, SIGNAL(clicked()), this, SLOT(OnLoadCoilRegistrationPoints()));

  connect(mControls.pbRoundPoints,SIGNAL(clicked()), this, SLOT(OnRoundPoints()));

  // Configure panel
  //mControls.PointsBox->setEnabled(false);

  // Interaction
  connect(mInteractor, SIGNAL(MoveCrossHair(mitk::Point3D)), this, SLOT(MoveCrossHair(mitk::Point3D)));

  // Hide 3d crosshair
  QmitkRenderWindow* threeD = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("3d");
  ViewCommands::Set3DCrosshairVisibility(false,GetDataStorage(),threeD);

  // error message
  mControls.lblError->setStyleSheet("color: rgb(255, 0, 0);");
  mControls.lblError->setVisible(false);

  // Point groups
  PointGroup* group1 = new PointGroup(1,PointGroup::GroupType::WithoutError);
  group1->pbSet = mControls.pbSetP1;
  group1->pbRemove = mControls.pbRemoveP1;
  group1->pbShow = mControls.pbShowP1;
  group1->lblName = mControls.lblP1;
  mPointGroupStack.pushNewPoint(group1);
  PointGroup* group2 = new PointGroup(2);
  group2->pbSet = mControls.pbSetP2;
  group2->pbRemove = mControls.pbRemoveP2;
  group2->pbShow = mControls.pbShowP2;
  group2->lblName = mControls.lblP2;
  mPointGroupStack.pushNewPoint(group2);
  PointGroup* group3 = new PointGroup(3);
  group3->pbSet = mControls.pbSetP3;
  group3->pbRemove = mControls.pbRemoveP3;
  group3->pbShow = mControls.pbShowP3;
  group3->lblName = mControls.lblP3;
  mPointGroupStack.pushNewPoint(group3);
  PointGroup* group4 = new PointGroup(4);
  group4->pbSet = mControls.pbSetP4;
  group4->pbRemove = mControls.pbRemoveP4;
  group4->pbShow = mControls.pbShowP4;
  group4->lblName = mControls.lblP4;
  mPointGroupStack.pushNewPoint(group4);
  PointGroup* group5 = new PointGroup(5);
  group5->pbSet = mControls.pbSetP5;
  group5->pbRemove = mControls.pbRemoveP5;
  group5->pbShow = mControls.pbShowP5;
  group5->lblName = mControls.lblP5;
  mPointGroupStack.pushNewPoint(group5);

}

void RegistrationPlanningView::SetFocus()
{
  //mControls.pbAddPoint->setFocus();
}

void RegistrationPlanningView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  if ( !GetSite()->GetPage()->IsPartVisible(GetSite()->GetPage()->FindView(GetSite()->GetId())) )
    return;

  if (nodes.empty() || (nodes.size() > 1))
  {
    mControls.pbStoreCoilPoints->setEnabled(false);
    mControls.pbLoadCoilPoints->setEnabled(false);

    mControls.pbRoundPoints->setEnabled(false);
    return;
  }

  auto node = *nodes.begin();

  // avoid changing the camera view if the selected node is the same as before
  if (mRegistrationImageNode == node)
    return;

  bool canUse=false;
  node->GetPropertyValue("navCAS.planning.useNode",canUse);

  // volume selected
  mitk::Image* Im = dynamic_cast<mitk::Image*>(node->GetData());
  if (Im != nullptr)
  {
    //mControls.PointsBox->setVisible(canUse);
    mControls.lblError->setVisible(!canUse);

    if (canUse)
      SetRegistrationNode(node);
    else
      mControls.lblError->setText(QString("Image was not selected for use in planning"));
  }

  // surface selected
  mitk::Surface* surf = dynamic_cast<mitk::Surface*>(node->GetData());
  if (surf != nullptr)
  {
    //mControls.PointsBox->setVisible(canUse);
    mControls.lblError->setVisible(!canUse);

    // Hide all surfaces but the selected one and perform global reinit
    if (canUse)
      SetRegistrationNode(node);

    // If selecting instrument, allow registration planning of it
    bool isInstrument=false;
    node->GetPropertyValue("navCAS.isInstrument",isInstrument);
    if (isInstrument)
    {
      //mControls.PointsBox->setVisible(true);
      mControls.lblError->setVisible(false);
      SetRegistrationNode(node);
    }
  }
}

void RegistrationPlanningView::MoveCrossHair(const mitk::Point3D position)
{
  GetRenderWindowPart()->SetSelectedPosition(position);
}


void RegistrationPlanningView::SetPoint(unsigned int pos, const mitk::Point3D point)
{
  mRegistrationPoints->InsertPoint(pos,point);

  // Store point in image node
  std::stringstream propName, propUseName;
  propName << "navCAS.registration.p" << pos+1;
  mRegistrationImageNode->SetProperty(propName.str().c_str(),mitk::Point3dProperty::New(point));
  propUseName << "navCAS.registration.useP" << pos+1;
  mRegistrationImageNode->SetBoolProperty(propUseName.str().c_str(),true);

  mRegistrationPointsNode->SetVisibility(true);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void RegistrationPlanningView::RemovePoint(unsigned int pos)
{
  mRegistrationPoints->RemovePointIfExists(pos);

  // Store point in image node
  std::stringstream propUseName;
  propUseName << "navCAS.registration.useP" << pos+1;
  mRegistrationImageNode->SetBoolProperty(propUseName.str().c_str(),false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void RegistrationPlanningView::SetRegistrationNode(mitk::DataNode::Pointer node)
{
  mRegistrationImageNode = node;
  node->SetBoolProperty("navCAS.canUseForRegistration",true);

  // ensure 3d axis is moved before applying global reinit
  mInteractor->SetPosition(node->GetData()->GetGeometry()->GetCenter());
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

  mNodesManager->ShowSingleSerie(node);

  // show only valid points in box
  //mControls.PointsBox->setEnabled(true);
  QString prop("navCAS.registration.useP");
  for (int i=0; ; i++)
  {
    bool show = false;
    if (!mRegistrationImageNode->GetBoolProperty((prop+QString::number(i+1)).toStdString().c_str(),show))
    {
      // remove any existing extra windows apart from the original 5
      while ((mPointGroupStack.size() > 5) &&
             (mPointGroupStack.size() > i))
      {
        cout << "Deleted Last point window" << std::endl;
        mPointGroupStack.removeLastPoint();
      }
      break;
    }

    if (i >= mPointGroupStack.size())
    {
      // create new window
      OnAddPoint();

      // add point in view
      mitk::Point3D point;
      mRegistrationImageNode->GetPropertyValue(("navCAS.registration.p" + QString::number(i+1)).toStdString().c_str(),point);
      mRegistrationPoints->InsertPoint(i,point);
    }

    mPointGroupStack[i]->pbShow->setEnabled(show);
    mPointGroupStack[i]->pbRemove->setEnabled(show);
  }

  mControls.pbRoundPoints->setEnabled(true);
  UpdateCoordinatesTable();
}

void RegistrationPlanningView::OnSetPoint()
{
  int pointNumber = mPointGroupStack.findPointGroupFromSender(sender(),PointGroupStack::SetPoint);
  cout << "Set point " << pointNumber << std::endl;

  int pos = pointNumber-1;

  SetPoint(pos,mInteractor->GetPosition());
  mPointGroupStack[pos]->pbShow->setEnabled(true);
  mPointGroupStack[pos]->pbRemove->setEnabled(true);
  UpdateCoordinatesTable();
}

// Remove registration points
void RegistrationPlanningView::OnRemovePoint()
{
  int pointNumber = mPointGroupStack.findPointGroupFromSender(sender(),PointGroupStack::RemovePoint);
  cout << "Remove point " << pointNumber << std::endl;

  int pos = pointNumber-1;

  RemovePoint(pos);
  mPointGroupStack[pos]->pbShow->setEnabled(false);
  mPointGroupStack[pos]->pbRemove->setEnabled(false);
  UpdateCoordinatesTable();
}

// Show selected registration point
void RegistrationPlanningView::OnShowPoint()
{
  int pointNumber = mPointGroupStack.findPointGroupFromSender(sender(),PointGroupStack::ShowPoint);
  cout << "Show point " << pointNumber << std::endl;

  int pos = pointNumber-1;

  ShowPoint(pos);
}

// Show registration points
void RegistrationPlanningView::ShowPoint(unsigned int p)
{
  mInteractor->SetPosition(mRegistrationPoints->GetPoint(p));
  MoveCrossHair(mRegistrationPoints->GetPoint(p));

  float zommFac = mControls.cbPrecZoom->isChecked()? 0.005 : 0.1;

  auto rws = GetRenderWindowPart()->GetQmitkRenderWindows();
  for (auto rw : rws)
  {
    // map point to 2D
    mitk::Point2D pos2D;
    rw->GetRenderer()->GetCurrentWorldPlaneGeometry()->Map(mRegistrationPoints->GetPoint(p),pos2D);

    auto cameraContoller = rw->GetRenderer()->GetCameraController();
    rw->GetRenderer()->SetConstrainZoomingAndPanning(false);
    cameraContoller->MoveCameraToPoint(pos2D);
    cameraContoller->SetScaleFactorInMMPerDisplayUnit(zommFac);
  }

  cout << "postion: " << mRegistrationPoints->GetPoint(p) << std::endl;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void RegistrationPlanningView::OnStoreCoilRegistrationPoints()
{
#ifdef WIN32
  string file("../");
#else
  string file("");
#endif // WIN32

  file += string("../data/coilRegistrationPoints.txt");

  std::ofstream data;
  data.open( file.c_str() );

  for (int p=0; p<mRegistrationPoints->GetSize(); p++)
  {
    data  << mRegistrationPoints->GetPoint(p)[0] << ";"
          << mRegistrationPoints->GetPoint(p)[1] << ";"
          << mRegistrationPoints->GetPoint(p)[2] << std::endl;
  }

  data.close();
}


void RegistrationPlanningView::OnLoadCoilRegistrationPoints()
{
#ifdef WIN32
  string file("../");
#else
  string file("");
#endif // WIN32

  file += string("../data/coilRegistrationPoints.txt");

  std::ifstream input(file.c_str());

  if (!input.good())
  {
    NodesManager::ErrorMessage("No coil registration points file found");
    return;
  }

  // each line is a valid 3d point
  unsigned int p=0;
  string line;
  while (getline(input,line))
  {
    mitk::Point3D point;
    stringstream  lineStream(line);
    string cell;
    getline(lineStream, cell, ';');
    point[0] = atof(cell.c_str());
    getline(lineStream, cell, ';');
    point[1] = atof(cell.c_str());
    getline(lineStream, cell, '\n');
    point[2] = atof(cell.c_str());

    SetPoint(p++,point);
    //mControls.pbShowP4->setEnabled(true);
  }

  input.close();
  return;
}

void RegistrationPlanningView::Activated()
{
  auto datanode = mInteractor->GetDataNode();
  if (datanode != nullptr)
    datanode->SetVisibility(true);

  mInteractor->ShowArrow(true);
}

void RegistrationPlanningView::Hidden()
{
  auto datanode = mInteractor->GetDataNode();
  if (datanode != nullptr)
    datanode->SetVisibility(false);

  mInteractor->ShowArrow(false);
  mNodesManager->ShowPlannedPoints(false);
}

void RegistrationPlanningView::OnAddPoint()
{
  // add new point to stack
  mPointGroupStack.pushNewPoint();

  // add connections
  connect(mPointGroupStack.back()->pbSet, SIGNAL(clicked()), this, SLOT(OnSetPoint()));
  connect(mPointGroupStack.back()->pbRemove, SIGNAL(clicked()), this, SLOT(OnRemovePoint()));
  connect(mPointGroupStack.back()->pbShow, SIGNAL(clicked()), this, SLOT(OnShowPoint()));

  // append to current gui layout
  mControls.PointsLayout->addWidget(mPointGroupStack.back()->window);
}

void RegistrationPlanningView::UpdateCoordinatesTable()
{
  const unsigned int nPoints = mPointGroupStack.size();
  mControls.twPointsCoordinates->setColumnCount(3);
  mControls.twPointsCoordinates->setRowCount(nPoints);
  QStringList header;
  header.push_back("X");
  header.push_back("Y");
  header.push_back("Z");
  mControls.twPointsCoordinates->setHorizontalHeaderLabels(header);
  for (unsigned int i=0; i<nPoints; ++i)
  {
    auto point = mRegistrationPoints->GetPoint(i);

    for (int x=0; x<3;++x)
      mControls.twPointsCoordinates->setItem(i,x,new QTableWidgetItem(QString::number(point[x])));
  }
}

void RegistrationPlanningView::OnRoundPoints()
{
  for (int id=0; id<mRegistrationPoints->GetSize(); ++id)
  {
    mitk::Point3D roundedPoint;
    if (!mRegistrationPoints->GetPointIfExists(id,&roundedPoint))
      continue;

    for (int i=0; i<3; ++i)
      roundedPoint[i] = static_cast<double>(std::round((roundedPoint[i]+0.01)*2))/2.0;

    SetPoint(id,roundedPoint);
  }

  mNodesManager->ShowPlannedPoints(true,mRegistrationImageNode);
  UpdateCoordinatesTable();
}
