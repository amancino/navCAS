/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>

// Qt
#include <QMessageBox>
#include <QDesktopWidget>
#include <QProgressBar>
#include <QSound>
#include <QSoundEffect>
#include <QString>

// Vtk
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkTransformPolyDataFilter.h>

// Mitk
#include <mitkDataNode.h>
#include <mitkSurface.h>
#include <mitkImage.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLayoutAnnotationRenderer.h>


#include <navAPI.h>
#include "NavRegView.h"
#include "Registration.h"
#include "SurfaceRefinement.h"
#include "IOCommands.h"

using namespace std;

// Don't forget to initialize the VIEW_ID.
const std::string NavRegView::VIEW_ID = "navcas.registration";

const double NavRegView::MAX_TOLERATED_PRIMARY_ERROR = 10.0;
const double NavRegView::MAX_TOLERATED_TOTAL_ERROR = 2.0;

NavRegView::NavRegView() :
  mIsNavigating(false),
  mAcceptedRegistration(false),
  mAcceptedPrimaryRegistration(false),
  mSetupIsConfigured(false)
{
  mNodesManager->CreatePlannedPoints();
  mNodesManager->CreatePrimaryRealPoints();
  mNodesManager->CreateSecondaryPoints();

  mPrimaryRegistrationTransformation = vtkSmartPointer<vtkMatrix4x4>::New();
  mPrimaryRegistrationTransformation->Identity();
  mSecondaryRegistrationTransformation = vtkSmartPointer<vtkMatrix4x4>::New();
  mSecondaryRegistrationTransformation->Identity();

  //mAddSound = new QSound(":/navcas.plugins/resources/add.wav");

  // TO DO: search in datastorage if points are present and load them
  // to do: nodes might be removed when closing the project (but not the view)
  if (mPrimaryRealPointsetDisplay.IsNull())
  {
    mPrimaryRealPointsetDisplay = mitk::DataNode::New();
    mPrimaryRealPointsetDisplay->SetName("Transformed real points (blue)");
    mPrimaryRealPointsetDisplay->SetFloatProperty("pointsize",3.0);
    mPrimaryRealPointsetDisplay->SetColor(0,0,1);
    mPrimaryRealPointsetDisplay->SetBoolProperty("helper object",!NodesManager::GetShowHelperObjects());
  }

  if (mSecondaryGreenPointsetDisplay.IsNull())
  {
    mSecondaryGreenPointsetDisplay = mitk::DataNode::New();
    mSecondaryGreenPointsetDisplay->SetName("Transformed secondary points (green)");
    mSecondaryGreenPointsetDisplay->SetFloatProperty("pointsize",3.0);
    mSecondaryGreenPointsetDisplay->SetColor(0,1,0);
    mSecondaryGreenPointsetDisplay->SetBoolProperty("helper object",!NodesManager::GetShowHelperObjects());
  }
  if (mSecondaryRedPointsetDisplay.IsNull())
  {
    mSecondaryRedPointsetDisplay = mitk::DataNode::New();
    mSecondaryRedPointsetDisplay->SetName("Transformed secondary points (red)");
    mSecondaryRedPointsetDisplay->SetFloatProperty("pointsize",3.0);
    mSecondaryRedPointsetDisplay->SetColor(1,0,0);
    mSecondaryRedPointsetDisplay->SetBoolProperty("helper object",!NodesManager::GetShowHelperObjects());
  }

  // registration error annotation
  for (unsigned int i=0; i<4; i++)
  {
    mRegistrationErrorLabel.push_back(mitk::TextAnnotation2D::New());
    mRegistrationErrorLabel[i]->SetFontSize(18);
    mRegistrationErrorLabel[i]->SetText("");
    mRegistrationErrorLabel[i]->SetOpacity(1);
    mRegistrationErrorLabel[i]->SetColor(1,0,0);
    // The LayoutAnnotationRenderer can place the TextAnnotation2D at some defined corner positions
    mitk::LayoutAnnotationRenderer::AddAnnotation(mRegistrationErrorLabel[i], (QString("stdmulti.widget")+QString::number(i)).toStdString(), mitk::LayoutAnnotationRenderer::BottomLeft, 5, 15, 1);
  }

//  cout << "Reference marker: " << mNodesManager->GetSetupReferenceMarker() << std::endl;
//  cout << "Moving marker: " << mNodesManager->GetSetupMovingMarker() << std::endl;
}

void NavRegView::RenderWindowPartActivated(mitk::IRenderWindowPart*)
{
  // updates the render window in NodesManager
  CheckValidRenderWindow();
}

void NavRegView::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{

}

void NavRegView::UpdateAnnotationMessage(string message, bool vis)
{
  for (unsigned int i=0; i<4; i++)
  {
    mRegistrationErrorLabel[i]->SetText(message);
    mRegistrationErrorLabel[i]->SetVisibility(vis);
  }
}

NavRegView::~NavRegView()
{
  cout << "NavRegView destructor" << std::endl;
  StopRegistration();
}

void NavRegView::CreateQtPartControl(QWidget* parent)
{
  // Setting up the UI is a true pleasure when using .ui files, isn't it?
  mControls.setupUi(parent);

  // GUI
  connect(mControls.pbStartPatientRegistration, SIGNAL(clicked()), this, SLOT(OnStartPatientRegistration()));
  connect(mControls.pbStartInstrumentRegistration, SIGNAL(clicked()), this, SLOT(OnStartInstrumentRegistration()));
  connect(mControls.pbCancelRegistration, SIGNAL(clicked()),this,SLOT(OnCancelRegistration()));
  connect(mControls.pbAcceptRegistration, SIGNAL(clicked()),this,SLOT(OnAcceptRegistration()));

  // primary registration
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

  connect(mControls.pbAcceptPrimary, SIGNAL(clicked()), this, SLOT(OnAcceptPrimaryRegistration()));
  connect(mControls.pbCancelPrimary, SIGNAL(clicked()), this, SLOT(OnCancelPrimaryRegistration()));

  // Build pointer vector to add point buttons
  // Point groups
  PointGroup* group1 = new PointGroup(1);
  group1->pbSet = mControls.pbSetP1;
  group1->pbRemove = mControls.pbRemoveP1;
  group1->lblName = mControls.lblP1;
  group1->lblError = mControls.lblErrorP1;
  group1->pbShow->setVisible(false);
  mPointGroupStack.pushNewPoint(group1);
  PointGroup* group2 = new PointGroup(2);
  group2->pbSet = mControls.pbSetP2;
  group2->pbRemove = mControls.pbRemoveP2;
  group2->lblName = mControls.lblP2;
  group2->lblError = mControls.lblErrorP2;
  group2->pbShow->setVisible(false);
  mPointGroupStack.pushNewPoint(group2);
  PointGroup* group3 = new PointGroup(3);
  group3->pbSet = mControls.pbSetP3;
  group3->pbRemove = mControls.pbRemoveP3;
  group3->lblName = mControls.lblP3;
  group3->lblError = mControls.lblErrorP3;
  group3->pbShow->setVisible(false);
  mPointGroupStack.pushNewPoint(group3);
  PointGroup* group4 = new PointGroup(4);
  group4->pbSet = mControls.pbSetP4;
  group4->pbRemove = mControls.pbRemoveP4;
  group4->lblName = mControls.lblP4;
  group4->lblError = mControls.lblErrorP4;
  group4->pbShow->setVisible(false);
  mPointGroupStack.pushNewPoint(group4);
  PointGroup* group5 = new PointGroup(5);
  group5->pbSet = mControls.pbSetP5;
  group5->pbRemove = mControls.pbRemoveP5;
  group5->lblName = mControls.lblP5;
  group5->lblError = mControls.lblErrorP5;
  group5->pbShow->setVisible(false);
  mPointGroupStack.pushNewPoint(group5);

  // secondary registration
  connect(mControls.pbAddPoint, SIGNAL(clicked()), this, SLOT(OnAddPoint()));
  connect(mControls.pbPerformSurfaceRefinement, SIGNAL(clicked()), this,  SLOT(OnPerformSurfaceRefinement()));
  connect(mControls.pbAcceptSecondary, SIGNAL(clicked()), this, SLOT(OnAcceptSecondary()));
  connect(mControls.pbCancelSecondary, SIGNAL(clicked()), this, SLOT(OnCancelSecondary()));

  // error messages
  mControls.lblErrorInfo->setVisible(false);
  mControls.lblAcceptInfo->setVisible(false);

  // add sound effects
  ConnectSoundEffects();

  // Navigation API
	ConnectNavigationActionsAndFunctions();

  CheckValidRenderWindow();
}

// https://www.soundjay.com/button-sounds-1.html
void NavRegView::ConnectSoundEffects()
{
  // add point
  QSoundEffect* addPoint = new QSoundEffect;
  addPoint->setSource(QUrl::fromLocalFile(":/navcas.navigationPlugins/resources/add.wav"));
  addPoint->setVolume(1.0);

  // remove point
  QSoundEffect* removePoint = new QSoundEffect;
  removePoint->setSource(QUrl::fromLocalFile(":/navcas.navigationPlugins/resources/remove.wav"));
  removePoint->setVolume(1.0);
  for (int i=0; i<mPointGroupStack.size(); i++)
  {
    connect(mPointGroupStack[i]->pbSet,SIGNAL(clicked()), addPoint, SLOT(play()) );
    connect(mPointGroupStack[i]->pbRemove,SIGNAL(clicked()), removePoint, SLOT(play()) );
  }

  // error sound
  mErrorSound = new QSoundEffect;
  mErrorSound->setSource(QUrl::fromLocalFile(":/navcas.navigationPlugins/resources/error.wav"));
  mErrorSound->setVolume(1.0);
}

void NavRegView::OnAcceptPrimaryRegistration()
{
  // just in case, perform registration
  PerformPrimaryRegistration();

  // enable secondary registration
  mAcceptedPrimaryRegistration = true;

  // disable the addition/removal of points
  for (int i=0; i<mPointGroupStack.size(); i++)
    mPointGroupStack[i]->pbRemove->setEnabled(false);

  // gui
  mControls.pbCancelPrimary->setEnabled(false);
  mControls.pbAcceptPrimary->setEnabled(false);
  mControls.pbPerformSurfaceRefinement->setEnabled(false);
  mControls.pbAcceptRegistration->setEnabled(true);

  // hide primary points
  ShowPrimaryRegistrationPoints(false);

  // store planned and real points in local database
  IOCommands::StoreRegistrationPoints(mRegistrationData);
}

bool NavRegView::OnCancelPrimaryRegistration()
{
  // warn the user what he is about to do!
  QMessageBox msgBox;
  msgBox.setText("Are you sure you want to cancel registration?");
  msgBox.setInformativeText("Registered points will be lost");
  msgBox.addButton("Cancel registration",QMessageBox::ButtonRole::AcceptRole);
  msgBox.addButton("Don't cancel registration",QMessageBox::ButtonRole::RejectRole);
  msgBox.setIcon(QMessageBox::Warning);
  int ret = msgBox.exec();

  cout << "Ret: " << ret << std::endl;
  if (ret)
    return false;

  mAcceptedPrimaryRegistration = false;

  // remove all added points
  for (int i=0; i<mPointGroupStack.size(); i++)
  {
    mNodesManager->SetUseRealPoint(i,false);
    mPointGroupStack[i]->lblError->setText("");
    mPointGroupStack[i]->pbRemove->setEnabled(false);
  }
  mControls.lblError->setText("nd");

  // stop navigation
  StopRegistration();

  // gui
  mControls.pbCancelPrimary->setEnabled(false);
  mControls.pbAcceptPrimary->setEnabled(false);

  // hide buttons
  OnInvalidProbeInView();

  return true;
}

void NavRegView::OnAcceptSecondary()
{
  mControls.pbCancelSecondary->setEnabled(false);
  mControls.pbAcceptSecondary->setEnabled(false);

  // hide secondary points
  ShowSecondaryRegistrationPoints(false);
}

void NavRegView::OnCancelSecondary()
{
  // remove secondary transformation
  mSecondaryRegistrationTransformation->Identity();
  mRegistrationTransformation = mPrimaryRegistrationTransformation;

  ClearSecondaryPoints();
  ShowRegistrationPoints(false);

  PerformPrimaryRegistration();

  mControls.pbCancelSecondary->setEnabled(false);
  mControls.pbAcceptSecondary->setEnabled(false);
}


void NavRegView::ShowRegistrationPoints(bool show)
{
  ShowPrimaryRegistrationPoints(show);
  ShowSecondaryRegistrationPoints(show);
}

void NavRegView::ShowPrimaryRegistrationPoints(bool show)
{
  if (mPrimaryRealPointsetDisplay.IsNotNull())
    mPrimaryRealPointsetDisplay->SetVisibility(show);

  mNodesManager->ShowPlannedPoints(show,mRegistrationSeries);
}

void NavRegView::ShowSecondaryRegistrationPoints(bool show)
{
  mSecondaryGreenPointsetDisplay->SetVisibility(show);
  mSecondaryRedPointsetDisplay->SetVisibility(show);
  mNodesManager->SetSecondaryPointsVisibility(show);
}

void NavRegView::ClearSecondaryPoints()
{
  mNodesManager->ClearSecondaryPoints();
  auto greenPs = dynamic_cast<mitk::PointSet*>(mSecondaryGreenPointsetDisplay->GetData());
  auto redPs = dynamic_cast<mitk::PointSet*>(mSecondaryRedPointsetDisplay->GetData());

  if (greenPs != nullptr)
    greenPs->Clear();
  if (redPs != nullptr)
    redPs->Clear();

  mControls.lblNumberSecondaryPoints->setText(QString::number(0));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void NavRegView::SetFocus()
{
  mControls.pbStartPatientRegistration->setFocus();
}

void NavRegView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  if ( !GetSite()->GetPage()->IsPartVisible(GetSite()->GetPage()->FindView(GetSite()->GetId())) )
    return;

  mControls.pbStartPatientRegistration->setEnabled(false);
  mControls.pbStartInstrumentRegistration->setEnabled(false);

  if (nodes.size() != 1)
    return;

  auto node = *nodes.begin();

  // If node is a registration image enable to start registration
  bool isRegistrationImage = false;
  node->GetBoolProperty("navCAS.canUseForRegistration",isRegistrationImage);

  if (isRegistrationImage && !mIsNavigating && mSetupIsConfigured)
  {
    bool isInstrument = false;
    node->GetBoolProperty("navCAS.isInstrument",isInstrument);

    mControls.pbStartInstrumentRegistration->setEnabled(isInstrument);
    mControls.pbStartPatientRegistration->setEnabled(!isInstrument);

    mSelectedSeries = node;
    mNodesManager->ShowSingleSerie(node);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


bool NavRegView::SetCurrentRegistrationSeries(mitk::DataNode::Pointer node, bool ChangeThroughGUI)
{
  if (node.IsNull())
  {
    cout << "No image selected!" << std::endl;
    return false;
  }

  // update registration series node
  mRegistrationSeries = node;

  bool isInstrument = false;
  mRegistrationSeries->GetBoolProperty("navCAS.isInstrument",isInstrument);
  if (!isInstrument)
  {
    string propName("navCAS.registration.isReferencePatientImage");

    // search posible existing current registration images and set to false the property
    mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New(propName.c_str(),mitk::BoolProperty::New(true));
    mitk::DataStorage::SetOfObjects::ConstPointer so = GetDataStorage()->GetSubset(pred);
    for (mitk::DataStorage::SetOfObjects::ConstIterator it = so->Begin(); it != so->End(); ++it)
      it->Value()->SetBoolProperty(propName.c_str(),false);

    // update current registration image bool property
    mRegistrationSeries->SetBoolProperty(propName.c_str(),true);
  }

  unsigned int validPoints = mNodesManager->ShowPlannedPoints(true, mRegistrationSeries);
  if (validPoints < 3)
  {
    QMessageBox msgBox;
    msgBox.setText("Invalid number of registration points.");
    msgBox.setInformativeText("Return to registration planning and add at least 3 registration points.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return false;
  }

  mControls.lblSeries->setText(node->GetName().c_str());



  // enable points in gui
  //mControls.PointsBox->setEnabled(true);
  QString prop("navCAS.registration.useP");
  for (int i=0; ; i++)
  {
    bool show = false;
    if (!mRegistrationSeries->GetBoolProperty((prop+QString::number(i+1)).toStdString().c_str(),show))
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
      // create new widget
      // add new point to stack
      mPointGroupStack.pushNewPoint(PointGroup::GroupType::WithError);
      mPointGroupStack.back()->pbShow->setVisible(false);

      // add connections
      connect(mPointGroupStack.back()->pbSet, SIGNAL(clicked()), this, SLOT(OnSetPoint()));
      connect(mPointGroupStack.back()->pbRemove, SIGNAL(clicked()), this, SLOT(OnRemovePoint()));

      // append to current gui layout
      mControls.PointsLayout->addWidget(mPointGroupStack.back()->window);

      // add point in view
      mitk::Point3D point;
      mRegistrationSeries->GetPropertyValue(("navCAS.registration.p" + QString::number(i+1)).toStdString().c_str(),point);
    }

    mPointGroupStack[i]->pbRemove->setEnabled(show);
  }

  for (int i=0; i < mPointGroupStack.size(); i++)
    mPointGroupStack[i]->lblName->setEnabled(mNodesManager->GetUsePlannedPoint(node,i));

  vtkSmartPointer<vtkMatrix4x4> transform = mNodesManager->GetTransformFromNode(mRegistrationSeries);

  // TO DO: ask if user wishes to load stored registration
  mRegistrationSeries->SetBoolProperty("navCAS.registration.isRegistered",false);
  if (transform != nullptr)
  {
    if (ChangeThroughGUI)
    {
      QMessageBox msgBox;
      msgBox.setText("Patient image has a stored registration!");
      msgBox.setIcon(QMessageBox::Information);
      msgBox.exec();
    }

    // set registered property
    mRegistrationSeries->SetBoolProperty("navCAS.registration.isRegistered",true);
    mRegistrationSeries->GetFloatProperty("navCAS.registration.error",mMeanError);
    stringstream error;
    error << mMeanError << " mm";
    mControls.lblError->setText(error.str().c_str());
    mControls.pbStartPatientRegistration->setText(QString("Improve patient registration"));

    mPrimaryRegistrationTransformation = transform;
    mAcceptedRegistration = true;
  }
  else
  {
    mPrimaryRegistrationTransformation = vtkSmartPointer<vtkMatrix4x4>::New();
    mAcceptedRegistration = false;
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  return true;
}

void NavRegView::OnAddPoint()
{
  // Store the point (in primary coordinates)
  int id = mNodesManager->InsertNewSecondaryPoint(mProbeLastPosition);

  // Check that the point is separated at least 0.5mm from previous stored points
  if (id == -1)
  {
    // Add error sound
    mErrorSound->play();
    return;
  }

  // store point in image/surface node
  stringstream propName;
  propName << "navCAS.navReg.secondaryP" << id;
  mRegistrationSeries->SetProperty(propName.str().c_str(),mitk::Point3dProperty::New());

  stringstream numberPoints;
  numberPoints << id+1;
  mControls.lblNumberSecondaryPoints->setText(numberPoints.str().c_str());

  mControls.pbPerformSurfaceRefinement->setEnabled(id>30);
}

void NavRegView::OnValidProbeInView()
{
  mControls.pbAddPoint->setEnabled(mAcceptedPrimaryRegistration);

  for (int i=0; i<mPointGroupStack.size(); i++)
    mPointGroupStack[i]->pbSet->setEnabled(!mAcceptedPrimaryRegistration);
}

void NavRegView::OnInvalidProbeInView()
{
  mControls.pbAddPoint->setEnabled(false);

  for (int i=0; i<mPointGroupStack.size(); i++)
    mPointGroupStack[i]->pbSet->setEnabled(false);
}

void NavRegView::OnCancelRegistration()
{
  StopRegistration();

  if (mRegistrationSeries.IsNotNull())
    mRegistrationSeries->SetBoolProperty("navCAS.registration.isRegistered",false);

  mControls.lblError->setText("nd");
}

void NavRegView::StopRegistration()
{
  NavigationPluginBase::StopNavigation();

  // gui
  mControls.pbCancelRegistration->setEnabled(false);
  mControls.pbAcceptRegistration->setEnabled(false);
  mControls.pbPerformSurfaceRefinement->setEnabled(false);

  if (mRegistrationType == Instrument)
  {
    mControls.pbStartInstrumentRegistration->setText(QString("Start instrument registration"));
    mControls.pbStartInstrumentRegistration->setEnabled(true);
  }

  mIsNavigating = false;
  ShowRegistrationPoints(false);

  ClearSecondaryPoints();

  // restore registration matrix to identity
  mPrimaryRegistrationTransformation->Identity();
  mSecondaryRegistrationTransformation->Identity();
}

// to do: user real point should be stored in its corresponding registration series (not in the system setup node)
void NavRegView::ResetRegistration()
{
  mMoveCrosshair = false;
  mShowProbe = false;
  auto realPoints = mNodesManager->GetPrimaryRealPointset();

  for (int i=0;i<realPoints->GetSize();i++)
    mNodesManager->SetUseRealPoint(i,false);

  realPoints->Clear();

  mPrimaryRegistrationTransformation->Identity();
  mSecondaryRegistrationTransformation->Identity();
  mRegistrationTransformation->Identity();
}

void NavRegView::OnStartPatientRegistration()
{
  if (!StartRegistration(mSelectedSeries,mNodesManager->GetReferenceMarker(),RegistrationType::Patient))
    return;

  mControls.pbStartPatientRegistration->setEnabled(false);
}

void NavRegView::OnStartInstrumentRegistration()
{
  if (!StartRegistration(mSelectedSeries,mNodesManager->GetMovingMarker(),RegistrationType::Instrument))
    return;

  mControls.pbStartInstrumentRegistration->setEnabled(false);
}

bool NavRegView::StartRegistration(mitk::DataNode::Pointer series, navAPI::MarkerId referenceMarker, RegistrationType type)
{
  // set selected node as registration image
  if (!SetCurrentRegistrationSeries(series, true))
    return false;

  ResetRegistration();
  // TO DO: check number of initial points already set


  // Load system setup configuration
  if (referenceMarker == navAPI::MarkerId::None)
  {
    NodesManager::ErrorMessage("Invalid marker for reference image","Please configure system setup parameters");
    return false;
  }

  // to do: load secondary points, if exist
  ClearSecondaryPoints();
  mAcceptedPrimaryRegistration = false;
  mAcceptedRegistration = false;

  bool registrationLoaded = false;

  // check if a valid registration can be found in disk
  if (IOCommands::CheckRegistrationInDisk())
  {
    MITK_INFO << "Found registration in disk";
    QMessageBox msgBox;
    msgBox.setText("A valid registration was found in disk.");
    msgBox.setInformativeText("Do you want to load it?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No  | QMessageBox::Cancel);
    int ret = msgBox.exec();

    IOCommands::SeriesType IOType = (type == Patient)? IOCommands::SeriesType::Patient : IOCommands::SeriesType::Instrument;
    switch (ret) {
      case QMessageBox::Yes:
      {
        // to do: should store which node is the one being registered?
        auto transform = IOCommands::ReadRegistrationFromDisk(IOType);

        mPrimaryRegistrationTransformation = transform;
        mRegistrationTransformation = transform;

        cout << "Transform succesfully read!" << std::endl;

        // block primary registration (consider it accepted)
        OnAcceptPrimaryRegistration();
        mMoveCrosshair = true;
        mShowProbe = true;
        registrationLoaded = true;
        break;
      }

      case QMessageBox::No:
        // remove stored registration
        IOCommands::RemoveStoredRegistration(IOType);
        break;

      case QMessageBox::Cancel:
        return false;

      default:
        // should never be reached
        break;
    }
  }

  // start new registration
  NavigationType mode = NavigationType::RegistrationPatient;
  if (type == RegistrationType::Instrument)
    mode = NavigationType::RegistrationInstrument;

  WaitCursorOn();
  bool started = NavigationPluginBase::StartNavigation(false,mode);
  if (!started)
  {
    mControls.pbStartPatientRegistration->setEnabled(true);
    mControls.pbCancelRegistration->setEnabled(false);
    WaitCursorOff();
    return false;
  }
  WaitCursorOff();

  mRegistrationType = type;



  mControls.pbCancelRegistration->setEnabled(true);

  mIsNavigating = true;

  // if registration was loaded do not check primary points
  if (!registrationLoaded)
    CheckRegistrationPoints();

  return true;
}

void NavRegView::OnAcceptRegistration()
{
	// to do: should I actually stop navigation here?
	NavigationPluginBase::StopNavigation();

  mControls.pbCancelRegistration->setEnabled(false);
  mControls.pbAcceptRegistration->setEnabled(false);
  mControls.pbPerformSurfaceRefinement->setEnabled(false);

  // Get transformation matrix
  vtkSmartPointer<vtkMatrix4x4> result = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkMatrix4x4::Multiply4x4(mSecondaryRegistrationTransformation,mPrimaryRegistrationTransformation,result);

  // store registration information in node
  mRegistrationSeries->SetBoolProperty("navCAS.registration.isRegistered",true);
  mRegistrationSeries->SetFloatProperty("navCAS.registration.error",mMeanError);

  // store transformation for patient/instrument
  if (mRegistrationType == RegistrationType::Patient)
  {
    // store registration in patient node (for scene saving)
    mNodesManager->StoreTransformInNode(result,mRegistrationSeries);

    // store registration in external file (in case of a crash)
    IOCommands::StoreRegistrationInDisk(result,IOCommands::Patient);

    mControls.pbStartPatientRegistration->setText(QString("Improve patient registration"));
  }
  else if (mRegistrationType == RegistrationType::Instrument)
  {
    // transform instrument surface accordingly
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->SetMatrix(result);
    transform->Inverse();
    transform->Update();

    // update instrument surface
    vtkSmartPointer<vtkTransformPolyDataFilter> filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    filter->SetInputData(dynamic_cast<mitk::Surface*>(mRegistrationSeries->GetData())->GetVtkPolyData());
    filter->SetTransform(transform);
    filter->Update();

    mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("navCAS.isInstrument",mitk::BoolProperty::New(true));
    mitk::DataStorage::SetOfObjects::ConstPointer so = GetDataStorage()->GetSubset(pred);

    // bug? I shouldn't modify the instrument node (its the original piece B)
    dynamic_cast<mitk::Surface*>(so->Begin()->Value()->GetData())->SetVtkPolyData(filter->GetOutput());

    // store transformation for instrument
    mNodesManager->StoreTransformInNode(result,mRegistrationSeries);

    IOCommands::StoreRegistrationInDisk(result,IOCommands::Instrument);

    mControls.pbStartInstrumentRegistration->setText(QString("Improve instrument registration"));
  }
  mIsNavigating = false;
  mAcceptedRegistration = true;
  ShowRegistrationPoints(false);

  // reset gui
  for (int i=0; i<mPointGroupStack.size(); i++)
  {
    mPointGroupStack[i]->lblError->setText("nd");
    mPointGroupStack[i]->lblError->setEnabled(false);
  }
}


mitk::Point3D NavRegView::PrimaryToOriginalCoordinates(mitk::Point3D point)
{
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkMatrix4x4::Invert(mPrimaryRegistrationTransformation,matrix);

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->SetMatrix(matrix);
  transform->Update();

  return mitk::Point3D(transform->TransformDoublePoint(point.GetDataPointer()));
}

void NavRegView::NewAveragedAcquisition(mitk::Point3D point, double sd, int)
{
  if (sd > 0.1)
  {
    cout << "sd too high for acquired point: " << sd << " mm" << std::endl;
    return;
  }

  mPointGroupStack[mCurrentPointId]->pbRemove->setEnabled(true);

  mNodesManager->SetPrimaryRealPoint(mCurrentPointId,PrimaryToOriginalCoordinates(point));
  mNodesManager->SetUseRealPoint(mCurrentPointId,true);
  CheckRegistrationPoints();
}

void NavRegView::OnSetPoint()
{
  int pointNumber = mPointGroupStack.findPointGroupFromSender(sender(),PointGroupStack::SetPoint);
  cout << "Set point " << pointNumber << std::endl;

  int pos = pointNumber-1;

  // store point id to use it later
  mCurrentPointId = pos;
  AverageProbeAcquisitions(10);
}

// Remove registration points
void NavRegView::OnRemovePoint()
{
  int pointNumber = mPointGroupStack.findPointGroupFromSender(sender(),PointGroupStack::RemovePoint);
  cout << "Remove point " << pointNumber << std::endl;

  int pos = pointNumber-1;

  mNodesManager->SetUseRealPoint(pos,false);
  mPointGroupStack[pos]->pbRemove->setEnabled(false);
  mPointGroupStack[pos]->lblError->setText("");
  CheckRegistrationPoints();
}



// **   REGISTRATION ALGORITHM  ** //

void NavRegView::CheckRegistrationPoints()
{
	unsigned int validPoints = 0;
  for (int i = 0; i < mNodesManager->GetPrimaryPlannedPointset()->GetSize(); i++)
	{
    if (mNodesManager->GetUseRealPoint(i))
    {
      mPointGroupStack[i]->pbRemove->setEnabled(true);
      mPointGroupStack[i]->lblError->setEnabled(true);
			validPoints++;
    }
	}

  cout << "Valid points: " << validPoints << std::endl;

  // update show probe
  mShowProbe = validPoints>=3;
  mMoveCrosshair = mShowProbe;

  double error = 10.0;
  if (mShowProbe)
    error = PerformRegistration();

  mNodesManager->SetShowProbe(mShowProbe);

  bool canAccept = error < MAX_TOLERATED_PRIMARY_ERROR;
  mControls.pbCancelPrimary->setEnabled(validPoints>0);
  mControls.pbAcceptPrimary->setEnabled(canAccept);
  mControls.lblAcceptInfo->setVisible(!canAccept);
}

double NavRegView::PerformRegistration()
{
  // Re-create the markers, in case of invalid nodes
  mNodesManager->CreateMarkers();

  double error = PerformPrimaryRegistration();

  mShowProbe = true;
  mMoveCrosshair = true;
  mControls.pbAddPoint->setEnabled(true);

  return error;
}

void NavRegView::OnPerformSurfaceRefinement()
{
  mControls.pbPerformSurfaceRefinement->setEnabled(false);

  PerformSecondaryRegistration();
  mShowProbe = true;
  mMoveCrosshair = true;

  mControls.pbAcceptSecondary->setEnabled(true);
  mControls.pbCancelSecondary->setEnabled(true);
}


double NavRegView::PerformPrimaryRegistration()
{
  // Get valid real and planned points
  unsigned int nRealPoints = mNodesManager->GetPrimaryRealPointset()->GetSize();
  cout << "Real points size: " << nRealPoints << std::endl;

  unsigned int nPlannedPoints = mNodesManager->GetPrimaryPlannedPointset()->GetSize();
  cout << "Planned points size: " << nPlannedPoints << std::endl;

  // if number of points < 3 it must not proceed
  if (nRealPoints < 3)
  {
    cout << "Not enough primary points" << std::endl;
    return -1.0;
  }

  // points placed using the probe
  mitk::PointSet::Pointer realPoints = mitk::PointSet::New();
  // planned points
  mitk::PointSet::Pointer plannedPoints = mitk::PointSet::New();
  // index list of used points
  std::vector<unsigned int> usedPlannedPoints;
  for (unsigned int i = 0; i < nPlannedPoints; i++)
	{
    if (mNodesManager->GetUseRealPoint(i))
    {
      realPoints->InsertPoint(mNodesManager->GetPrimaryRealPointset()->GetPoint(i));
      plannedPoints->InsertPoint(mNodesManager->GetPrimaryPlannedPointset()->GetPoint(i));
      usedPlannedPoints.push_back(i);
    }
	}

  // perform paired points registration
  mitk::PointSet::Pointer transformedRealPointset = mitk::PointSet::New();
  mPrimaryRegistrationTransformation = Registration::PerformPairedPointsRegistration(plannedPoints,realPoints,transformedRealPointset);

  //update current registration used for navigation
  mRegistrationTransformation = mPrimaryRegistrationTransformation;
  mSecondaryRegistrationTransformation->Identity();

  cout << "Primary registration: " << std::endl;
  for (int i=0; i<4; i++)
  {
    for (int j =0; j<4; j++)
      cout << mPrimaryRegistrationTransformation->GetElement(i,j) << " ";

    cout << std::endl;
  }

  // moved pointset
  mPrimaryRealPointsetDisplay->SetData(transformedRealPointset);
  if (!GetDataStorage()->Exists(mPrimaryRealPointsetDisplay))
    GetDataStorage()->Add(mPrimaryRealPointsetDisplay);

  // remove secondary points (if existing)
  mNodesManager->ClearSecondaryPoints();

  // The primary registration error is computed and informed
  std::vector<double> dist;
  double mean,std,fre,fle;
  Registration::GetErrorMetricsFromPairedPointRegistration(plannedPoints,transformedRealPointset,dist,mean,std,fre,fle);
  for(unsigned int i=0; i<dist.size(); ++i)
  {
    // update point error in gui
    stringstream pointError;
    pointError << round(dist[i]*10.0)/10.0 << " mm";

    mPointGroupStack[usedPlannedPoints[i]]->lblError->setText(pointError.str().c_str());
  }

  mControls.lblError->setText(QString::number(mean,'g',1) + "+-" + QString::number(std,'g',1) + " mm");
  mControls.lblFRE->setText(QString::number(fre,'g',2) + " mm");
  mControls.lblError->setEnabled(true);
  mControls.slblError->setEnabled(true);
  mControls.lblErrorInfo->setVisible(true);

  // store error
  mMeanError = mean;

  double informedError = round(mean*10)/10.0;

  if (informedError > MAX_TOLERATED_TOTAL_ERROR)
    UpdateAnnotationMessage("Bad registration!",true);
  else
    UpdateAnnotationMessage("",false);

  // show points
  ShowPrimaryRegistrationPoints(true);

  // store registration data in structure
  mRegistrationData.mOriginalRealPointsUsedInRegistration = realPoints;
  mRegistrationData.mTransformedRealPointsUsedInRegistration = transformedRealPointset;
  mRegistrationData.mPlannedPointsUsedInRegistration = plannedPoints;
  mRegistrationData.mRegistrationType = mRegistrationType == Patient? IOCommands::Patient : IOCommands::Instrument;

  return informedError;
}


void NavRegView::PerformSecondaryRegistration()
{
  // Get secondary points
  mitk::PointSet::Pointer secondaryPoints = mNodesManager->GetSecondaryPointset();

  if (secondaryPoints->GetSize() < 32)
  {
    cout << "Not enough points for surface refinement: only " << secondaryPoints->GetSize() << std::endl;
    return;
  }

  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("navCAS.canUseForRegistration",mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer so = GetDataStorage()->GetSubset(pred);

  if (so->size() < 1)
  {
    cout << "Add points to at least 1 image" << std::endl;
    return;
  }

  // Get planned patient surface
  // TO DO: put string in module
  mitk::DataNode::Pointer plannedSurf;
  if (dynamic_cast<mitk::Image*>(mRegistrationSeries->GetData()) != nullptr)
    plannedSurf = GetDataStorage()->GetNamedDerivedNode("navCAS_planning_surface",mRegistrationSeries);
  else if (dynamic_cast<mitk::Surface*>(mRegistrationSeries->GetData()) != nullptr)
    plannedSurf = mRegistrationSeries;


  // process in thread
  mSurfaceRefinementThread = new SurfaceRefinementThread;
  mSurfaceRefinementThread->setPointset(secondaryPoints);
  mSurfaceRefinementThread->SetSurfaceNode(plannedSurf);

  mProgressbar = new QProgressBar;
  mProgressbar->setValue(0);
  mProgressbar->setAlignment(Qt::AlignCenter);
  mProgressbar->setWindowModality(Qt::WindowModality::ApplicationModal);
  mProgressbar->setWindowTitle("Performing surface refinement...");
  mProgressbar->resize(300,40);
  //mProgressbar->setFormat(QString("Processing..."));
  mProgressbar->show();
  mProgressbar->move( ( QApplication::desktop()->width() - mProgressbar->width() ) / 2,
                      ( QApplication::desktop()->height() - mProgressbar->height() ) / 2);

  connect(mSurfaceRefinementThread, SIGNAL(percentageCompleted(int)), this, SLOT(OnUpdateSurfaceRefinementProcess(int)));
  connect(mSurfaceRefinementThread, SIGNAL(cancelationFinished()), this, SLOT(OnSurfaceRefinementCanceled()));
  connect(mSurfaceRefinementThread, SIGNAL(finished()), this, SLOT(OnSurfaceRefinementFinished()));

  mSurfaceRefinementThread->start();
}

void NavRegView::OnUpdateSurfaceRefinementProcess(int steps)
{
  cout << "New percentage: " << steps << std::endl;
  mProgressbar->setValue(steps);
}

void NavRegView::OnSurfaceRefinementFinished()
{
  mSecondaryGreenPointsetDisplay->SetData(mSurfaceRefinementThread->GetGreenPointSet());
  mSecondaryRedPointsetDisplay->SetData(mSurfaceRefinementThread->GetRedPointSet());

  // show secondary points
  if (!GetDataStorage()->Exists(mSecondaryGreenPointsetDisplay))
    GetDataStorage()->Add(mSecondaryGreenPointsetDisplay);
  if (!GetDataStorage()->Exists(mSecondaryRedPointsetDisplay))
    GetDataStorage()->Add(mSecondaryRedPointsetDisplay);

  // remove primary real points (no longer valid)
  auto realPs = dynamic_cast<mitk::PointSet*>(mPrimaryRealPointsetDisplay->GetData());
  if (realPs != nullptr)
    realPs->Clear();

  std::stringstream error;
  double totalError = mSurfaceRefinementThread->GetError();
  totalError = round(totalError*10.0)/10.0;
  error << totalError << " mm";
  mControls.lblError->setText(error.str().c_str());
  mControls.lblErrorInfo->setVisible(true);

  // store error
  mMeanError = totalError;

  if (mSurfaceRefinementThread->GetError() > MAX_TOLERATED_TOTAL_ERROR)
    UpdateAnnotationMessage("Bad registration!",true);
  else
    UpdateAnnotationMessage("",false);

  // Get resulting surface refinement registration matrix
  mSecondaryRegistrationTransformation = mSurfaceRefinementThread->GetOutputMatrix();

  cout << "Secondary registration: " << std::endl;
  for (int i=0; i<4; i++)
  {
    for (int j =0; j<4; j++)
      cout << mSecondaryRegistrationTransformation->GetElement(i,j) << " ";

    cout << std::endl;
  }

  //update current registration used for navigation
  vtkSmartPointer<vtkMatrix4x4> totalRegistrationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  totalRegistrationMatrix->Multiply4x4(mSecondaryRegistrationTransformation,mPrimaryRegistrationTransformation,totalRegistrationMatrix);
  mRegistrationTransformation = totalRegistrationMatrix;

  cout << std::endl << "Total registration: " << std::endl;
  for (int i=0; i<4; i++)
  {
    for (int j =0; j<4; j++)
      cout << mRegistrationTransformation->GetElement(i,j) << " ";

    cout << std::endl;
  }

  delete mSurfaceRefinementThread;
  delete mProgressbar;
}

void NavRegView::OnSurfaceRefinementCanceled()
{
  delete mSurfaceRefinementThread;
  delete mProgressbar;
}

void NavRegView::Activated()
{
  ShowRegistrationPoints(true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  // detect navigation mode
  QString navMode;
  auto mode = mNodesManager->GetNavigationMode();
  mSetupIsConfigured = true;
  switch (mode)
  {
  case NodesManager::NavigationMode::Traditional:
    navMode = "Traditional navigation";
    mControls.pbStartInstrumentRegistration->setVisible(false);
    break;
  case NodesManager::NavigationMode::InstrumentTracking:
    navMode = "Multiple markers navigation";
    mControls.pbStartInstrumentRegistration->setVisible(true);
    mControls.pbStartInstrumentRegistration->setText("Start instrument registration");
    break;
  default:
    navMode = "The system has not been configured yet!";
    mSetupIsConfigured = false;
  }

  if (mSetupIsConfigured)
    mControls.lblNavigationMode->setStyleSheet("color:green");
  else
    mControls.lblNavigationMode->setStyleSheet("color:red");

  mControls.lblNavigationMode->setText(navMode);
}

void NavRegView::Hidden()
{
  ShowRegistrationPoints(false);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
