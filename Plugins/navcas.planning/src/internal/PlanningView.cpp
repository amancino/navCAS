/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>

#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <usGetModuleContext.h>
#include <usModuleRegistry.h>

// itk
#include <itkBinaryThresholdImageFilter.h>

// vtk
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPolyDataNormals.h>

// qmitk
#include <QmitkRenderWindow.h>

// Mitk
#include <mitkDataNode.h>
#include <mitkSurface.h>
#include <mitkNodePredicateProperty.h>
#include <mitkImageCast.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkNodePredicateDataType.h>

#include "SurfaceFilter.h"
#include "PlanningView.h"
#include "SurfaceAdaptation.h"
#include "ViewCommands.h"
#include "LabeledPointSetMapper3D.h"
#include "LabeledPointSetMapper2D.h"

using namespace std;

const std::string PlanningView::VIEW_ID = "navcas.planning";
const std::string PlanningView::SURFACE_NAME = string("navCAS_planning_surface");

PlanningView::PlanningView() :
  mAttachmentCounter(0)
{
  mPreviewNode = mitk::DataNode::New();
  mPreviewNode->SetName("Preview");
  mPreviewNode->SetColor(1,0,0);
  mPreviewNode->SetBoolProperty("helper object",!NodesManager::GetShowHelperObjects());

  mNodesManager = new NodesManager(GetDataStorage());

  // Start simple interaction
  us::Module* module = us::GetModuleContext()->GetModule("Interactors");
  mInteractor = PlanningInteractor::New(GetDataStorage());
  mInteractor->LoadStateMachine("PlanningInteraction.xml", module);
  mInteractor->SetEventConfig("PlanningInteractionConfig.xml", module);

  CheckInteraction();
}


PlanningView::~PlanningView()
{
  delete mNodesManager;

  mitk::DataNode::Pointer node = mInteractor->GetDataNode();
  if (GetDataStorage()->Exists(node))
    GetDataStorage()->Remove(node);

  mInteractor->SetDataNode(nullptr);
}

void PlanningView::CreateQtPartControl(QWidget* parent)
{
  // Setting up the UI is a true pleasure when using .ui files, isn't it?
  mControls.setupUi(parent);
  connect(mControls.sliderThreshold, SIGNAL(valueChanged(int)), this, SLOT(OnSliderChanged()));
  connect(mControls.cbShow2D, SIGNAL(clicked()), this, SLOT(OnSetShow2D()));
  connect(mControls.cbUseNode, SIGNAL(clicked()), this, SLOT(OnUseNodeChanged()));

  // point measurements planning
  connect(mControls.pbStartPointPlanning, SIGNAL(clicked()), this, SLOT(OnStartPointPlanning()));
  connect(mControls.pbAddMeasurementPoint, SIGNAL(clicked()), this, SLOT(OnAddMeasurementPoint()));
  connect(mControls.pbLoadPointPlanning, SIGNAL(clicked()), this, SLOT(OnLoadPointPlanning()));

  // Store render windows
  QmitkRenderWindow* axial = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("axial");
  QmitkRenderWindow* coronal = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("coronal");
  QmitkRenderWindow* sagittal = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("sagittal");
  QmitkRenderWindow* threeD = GetRenderWindowPart(mitk::WorkbenchUtil::OPEN)->GetQmitkRenderWindow("3d");
  mRenderWindows.push_back(axial);
  mRenderWindows.push_back(coronal);
  mRenderWindows.push_back(sagittal);
  mRenderWindows.push_back(threeD);

  // Hide 3d crosshair
  ViewCommands::Set3DCrosshairVisibility(false,GetDataStorage(),threeD);

  // Interaction
  connect(mInteractor, SIGNAL(MoveCrossHair(mitk::Point3D)), this, SLOT(MoveCrossHair(mitk::Point3D)));

  // set axis in current crosshair position
  auto pos = GetRenderWindowPart()->GetSelectedPosition();
  mInteractor->SetPosition(pos);
}

void PlanningView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  mRenderWindows.clear();
  QmitkRenderWindow* axial = renderWindowPart->GetQmitkRenderWindow("axial");
  QmitkRenderWindow* coronal = renderWindowPart->GetQmitkRenderWindow("coronal");
  QmitkRenderWindow* sagittal = renderWindowPart->GetQmitkRenderWindow("sagittal");
  QmitkRenderWindow* threeD = renderWindowPart->GetQmitkRenderWindow("3d");
  mRenderWindows.push_back(axial);
  mRenderWindows.push_back(coronal);
  mRenderWindows.push_back(sagittal);
  mRenderWindows.push_back(threeD);

  // Hide 3d crosshair
  ViewCommands::Set3DCrosshairVisibility(false,GetDataStorage(),threeD);
}

void PlanningView::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{
  mRenderWindows.clear();
}

void PlanningView::CheckInteraction()
{
  mitk::DataNode::Pointer interactorNode = mInteractor->GetDataNode();
  if ((interactorNode == nullptr) || (!GetDataStorage()->Exists(interactorNode)))
  {
    interactorNode = mitk::DataNode::New();
    auto gd = mitk::GeometryData::New();
    interactorNode->SetData(gd);
    interactorNode->SetBoolProperty("helper object",!NodesManager::GetShowHelperObjects());
    interactorNode->SetName("Planning Interactor");
    interactorNode->SetVisibility(true);
    GetDataStorage()->Add(interactorNode);
    mInteractor->SetDataNode(interactorNode);
  }

  if (!GetDataStorage()->Exists(interactorNode))
    GetDataStorage()->Add(interactorNode);
}

void PlanningView::MoveCrossHair(mitk::Point3D position)
{
  GetRenderWindowPart()->SetSelectedPosition(position);
}

void PlanningView::OnSetShow2D()
{
  if (mRenderWindows.empty())
    return;

  mitk::DataNode::Pointer lowRes = SurfaceAdaptation::GetLowResolutionNode(GetDataStorage(),mSelectedNode);
  if (lowRes.IsNull())
    return;

  bool vis=true;
  mSelectedNode->GetBoolProperty("visible",vis);
  for (unsigned int i=0; i<3; i++)
    lowRes->SetVisibility(mControls.cbShow2D->isChecked()&&vis,mRenderWindows[i]->GetRenderer());

  // boolean controls the user desire to show or not the 2D slices
  mSelectedNode->SetBoolProperty("navCAS.surface.show2D",false);
  lowRes->SetBoolProperty("navCAS.surface.show2D",mControls.cbShow2D->isChecked());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void PlanningView::OnSliderChanged()
{
  stringstream value;
  value << mControls.sliderThreshold->value();
  mControls.lblThreshold->setText(value.str().c_str());

  // show preview of binary threshold
  mitk::Image::Pointer mitkImage = dynamic_cast<mitk::Image*>(mSelectedNode->GetData());

  if (mitkImage.IsNull())
    return;

  // Cast Mitk Image from node to Itk Image
  typedef itk::Image<short, 3> ImageType;
  ImageType::Pointer itkImage;
  mitk::CastToItkImage(mitkImage, itkImage);

  // Re-Register surface generated
  mitk::BaseGeometry::Pointer geom = mSelectedNode->GetData()->GetGeometry()->Clone();
  geom->ChangeImageGeometryConsideringOriginOffset(false);

  // Create new binary Itk Image containing only the preview
  typedef itk::BinaryThresholdImageFilter<ImageType, ImageType> BinaryThresholdImageFilter;

  BinaryThresholdImageFilter::Pointer BrainBinaryFilter = BinaryThresholdImageFilter::New();
  BrainBinaryFilter->SetInput(itkImage);
  BrainBinaryFilter->SetLowerThreshold(mControls.sliderThreshold->value());
  BrainBinaryFilter->SetUpperThreshold(32000);
  BrainBinaryFilter->SetInsideValue(1);
  BrainBinaryFilter->SetOutsideValue(0);
  BrainBinaryFilter->Update();

  // Cast back from Itk Image to Mitk Image
  mitk::Image::Pointer newMitkImagePreview = mitk::Image::New();
  mitk::Image::Pointer pv = mitk::Image::New();
  CastToMitkImage(BrainBinaryFilter->GetOutput(),pv);
  //newMitkImagePreview = mitk::ImportItkImage(BrainBinaryFilter->GetOutput());
  //newMitkImagePreview->SetGeometry(geom);
  pv->SetGeometry(geom);

  mPreviewNode->SetData(pv);
  mPreviewNode->SetVisibility(true);

  if (!GetDataStorage()->Exists(mPreviewNode))
    GetDataStorage()->Add(mPreviewNode);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void PlanningView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{  
  mControls.cbUseNode->setEnabled(false);
  mControls.cbUseNode->setChecked(false);
  mControls.cbShow2D->setEnabled(false);
  mControls.cbShow2D->setChecked(false);
  mControls.lblThreshold->setEnabled(false);
  mControls.sliderThreshold->setEnabled(false);
  mControls.pbLoadPointPlanning->setEnabled(false);
  mControls.pbStartPointPlanning->setEnabled(true);

  if (nodes.size() == 0)
    return;

  // Set visible only the selected node
  auto node = *nodes.begin();

  bool useForPlanning = false;
  node->GetBoolProperty("navCAS.planning.useNode",useForPlanning);
  mControls.cbUseNode->setChecked(useForPlanning);

  // volume is selected
  mitk::Image* Im = dynamic_cast<mitk::Image*>(node->GetData());
  if (Im != nullptr)
  {
    mSelectedNode = node;

    mControls.cbUseNode->setEnabled(true);
    mControls.lblThreshold->setEnabled(true);
    mControls.sliderThreshold->setEnabled(true);
    mControls.cbShow2D->setEnabled(false);
    node->SetVisibility(true);

    mitk::DataNode::Pointer skin = GetSkinNode(GetDataStorage(),node);
    if (skin.IsNotNull())
      skin->SetVisibility(true);

    return;
  }

  // surface is selected
  mitk::Surface* surf = dynamic_cast<mitk::Surface*>(node->GetData());
  if (surf != nullptr)
  {
    mSelectedNode = node;

    // If surf is not set to use for planning
    bool isInstrument=false;
    node->GetBoolProperty("navCAS.isInstrument",isInstrument);

    //use for planning checkbox
    mControls.cbUseNode->setEnabled(!isInstrument);
    mControls.cbUseNode->setChecked(useForPlanning || isInstrument);

    if (isInstrument)
    {
      mControls.lblInstrument->setText(node->GetName().c_str());
    }

    if (useForPlanning)
    {
      mitk::DataNode::Pointer lowRes = SurfaceAdaptation::GetLowResolutionNode(GetDataStorage(),node);
      if (lowRes.IsNull())
      {
        // trigger node changed for recomputation of the low resolution node
        OnUseNodeChanged();
        lowRes = SurfaceAdaptation::GetLowResolutionNode(GetDataStorage(),node);
      }

      mControls.cbShow2D->setEnabled(true);
      bool vis = true;
      lowRes->GetVisibility(vis,mRenderWindows[0]->GetRenderer());
      mControls.cbShow2D->setChecked(vis);
    }
    return;
  }

  // pointset
  auto ps = dynamic_cast<mitk::PointSet*>(node->GetData());
  if (ps != nullptr)
  {
    mControls.pbLoadPointPlanning->setEnabled(true);
    mControls.pbStartPointPlanning->setEnabled(false);
    mSelectedNode = node;
    return;
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void PlanningView::OnUseNodeChanged()
{
  bool use = mControls.cbUseNode->isChecked();
  mSelectedNode->SetBoolProperty("navCAS.planning.useNode",use);

  // Image
  auto im = dynamic_cast<mitk::Image*>(mSelectedNode->GetData());
  if (im != nullptr)
  {
    // Extract skin and automatically configure it in low resolution mode for 2D windows
    mSelectedNode = ExtractSkin(mSelectedNode);
    OnUseNodeChanged();
    return;
  }

  // Surface
  auto surf = dynamic_cast<mitk::Surface*>(mSelectedNode->GetData());
  if (surf != nullptr)
  {
    // Create low resolution Surface
    SurfaceAdaptation::AttachLowResolutionSurface(GetDataStorage(),mSelectedNode);
    mitk::DataNode::Pointer lowRes = SurfaceAdaptation::GetLowResolutionNode(GetDataStorage(),mSelectedNode);

    // In 2D only show the low resolution surface
    mitk::BaseRenderer* axial = GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetRenderer();
    mitk::BaseRenderer* coronal = GetRenderWindowPart()->GetQmitkRenderWindow("coronal")->GetRenderer();
    mitk::BaseRenderer* sagittal = GetRenderWindowPart()->GetQmitkRenderWindow("sagittal")->GetRenderer();
    mitk::BaseRenderer* threeD = GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderer();

    // restore default visibility
    mSelectedNode->GetPropertyList(axial)->DeleteProperty("visible");
    mSelectedNode->GetPropertyList(coronal)->DeleteProperty("visible");
    mSelectedNode->GetPropertyList(sagittal)->DeleteProperty("visible");
    mSelectedNode->GetPropertyList(threeD)->DeleteProperty("visible");

    // set new visibility behaviour
    mSelectedNode->SetVisibility(false,axial);
    mSelectedNode->SetVisibility(false,coronal);
    mSelectedNode->SetVisibility(false,sagittal);
    mSelectedNode->SetVisibility(true);
    lowRes->SetVisibility(true,axial);
    lowRes->SetVisibility(true,coronal);
    lowRes->SetVisibility(true,sagittal);
    lowRes->SetVisibility(false);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


mitk::DataNode::Pointer PlanningView::ExtractSkin(mitk::DataNode* parent)
{
  mitk::DataNode::Pointer surfNode = GetDataStorage()->GetNamedDerivedNode(SURFACE_NAME.c_str(),parent);
  if (surfNode.IsNull())
  {
    SurfaceFilter* filter = new SurfaceFilter;
    filter->SetInputImage(dynamic_cast<mitk::Image*>(parent->GetData()));
    filter->SetThreshold(mControls.sliderThreshold->value());
    filter->Update();

    // Set geometry and add to datastorage
    surfNode = mitk::DataNode::New();
    surfNode->SetData(filter->GetOutput());
    surfNode->SetName(SURFACE_NAME);
    surfNode->SetBoolProperty("helper object",!NodesManager::GetShowHelperObjects());
    surfNode->SetProperty("material.interpolation",mitk::VtkInterpolationProperty::New(0));  // flat interpolation

    if (!GetDataStorage()->Exists(surfNode))
      GetDataStorage()->Add(surfNode,parent);

    delete filter;
  }
  mPreviewNode->SetVisibility(false);


  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return surfNode;
}

mitk::DataNode::Pointer PlanningView::GetSkinNode(const mitk::DataStorage::Pointer ds, const mitk::DataNode::Pointer node)
{
  return ds->GetNamedDerivedNode(SURFACE_NAME.c_str(),node);
}


void PlanningView::NodeAdded(const mitk::DataNode *node)
{
  // to do: request observers attachment update
  auto surf = dynamic_cast<mitk::Surface*>(node->GetData());
  if (surf != nullptr)
  {
    // add visibility and color observer to node
    RequestUpdateAttachments();
  }
}

void PlanningView::NodeRemoved(const mitk::DataNode *node)
{
  auto surf = dynamic_cast<mitk::Surface*>(node->GetData());
  if (surf != nullptr)
  {
    auto nonConstNode = const_cast<mitk::DataNode*>(node);
    auto child = SurfaceAdaptation::GetLowResolutionNode(GetDataStorage(),nonConstNode);

    if (child == nullptr)
      return;

    // remove child node
    if (GetDataStorage()->Exists(child))
      GetDataStorage()->Remove(child);

    //nonConstNode->RemoveAllObservers();
  }
}

void PlanningView::RequestUpdateAttachments()
{
  mAttachmentCounter++;
  QTimer::singleShot(1000,this,SLOT(UpdateAttachments()));
}

void PlanningView::UpdateAttachments()
{
  if (--mAttachmentCounter > 0)
    return;

  auto pred = mitk::NodePredicateDataType::New("Surface");
  auto so = GetDataStorage()->GetSubset(pred);

  for (auto it = so->Begin(); it != so->End(); ++it)
  {
    auto node = it->Value();
    auto child = SurfaceAdaptation::GetLowResolutionNode(GetDataStorage(),node);
    if (child == nullptr)
    {
      //nonConstNode->SetBoolProperty("navCAS.observers.hasNodeObserver", false);
      continue;
    }

    AttachObservers(node);
  }

  CheckInteraction();
}

void PlanningView::AttachObservers(mitk::DataNode::Pointer node) const
{
  if (node.IsNull())
    return;

  cout << "Attaching observers to " << node->GetName() << std::endl;

  // Add observer of visibility change
  itk::MemberCommand<PlanningView>::Pointer visibilityModifiedCommand	= itk::MemberCommand<PlanningView>::New();
  visibilityModifiedCommand->SetCallbackFunction(const_cast<PlanningView*>(this), &PlanningView::VisibilityModified);
  node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), visibilityModifiedCommand);

  // Add change color observer
  itk::MemberCommand<PlanningView>::Pointer colorModifiedCommand = itk::MemberCommand<PlanningView>::New();
  colorModifiedCommand->SetCallbackFunction(const_cast<PlanningView*>(this), &PlanningView::ColorModified);
  node->GetProperty("color")->AddObserver(itk::ModifiedEvent(), colorModifiedCommand);

  node->SetBoolProperty("navCAS.observers.hasNodeObserver", true);
}

void PlanningView::VisibilityModified(const itk::Object *, const itk::EventObject &)
{
  auto ds = GetDataStorage();
  auto pred = mitk::NodePredicateDataType::New("Surface");
  auto so = ds->GetSubset(pred);
  for (auto it = so->Begin(); it != so->End(); ++it)
  {
    auto parent = it->Value();
    auto child = SurfaceAdaptation::GetLowResolutionNode(ds,it->Value());
    if (child == nullptr)
      continue;

    bool vis=false;
    parent->GetBoolProperty("visible",vis);
    bool show=true;
    child->GetBoolProperty("navCAS.surface.show2D",show);
    child->SetVisibility(false);
    for (int i=0; i<3; i++)
      child->SetVisibility(vis&&show,mRenderWindows[i]->GetRenderer());
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void PlanningView::ColorModified(const itk::Object *, const itk::EventObject &)
{
  cout << "Color changed " << std::endl;
  auto ds = GetDataStorage();
  auto pred = mitk::NodePredicateDataType::New("Surface");
  auto so = ds->GetSubset(pred);
  for (auto it = so->Begin(); it != so->End(); ++it)
  {
    auto parent = it->Value();
    auto child = SurfaceAdaptation::GetLowResolutionNode(ds,it->Value());
    if (child == nullptr)
      continue;

    float color[3];
    parent->GetColor(color);
    child->SetColor(color);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void PlanningView::Activated()
{
  CheckInteraction();
  mInteractor->GetDataNode()->SetVisibility(true);
  mInteractor->ShowArrow(true);
}

void PlanningView::Hidden()
{
  CheckInteraction();
  mInteractor->GetDataNode()->SetVisibility(false);
  mInteractor->ShowArrow(false);
}

void PlanningView::OnStartPointPlanning()
{
  mMeasurementPlanningPointset = mitk::PointSet::New();
  mMeasurementPointsNode = mitk::DataNode::New();
  mMeasurementPointsNode->SetData(mMeasurementPlanningPointset);
  mMeasurementPointsNode->SetName("Planned measurement points");
  GetDataStorage()->Add(mMeasurementPointsNode);

  LabeledPointSetMapper3D::Pointer mapper = LabeledPointSetMapper3D::New();
  mMeasurementPointsNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, mapper);

  LabeledPointSetMapper2D::Pointer mapper2D = LabeledPointSetMapper2D::New();
  mMeasurementPointsNode->SetStringProperty("default label","P");
  mMeasurementPointsNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard2D, mapper2D);

  mControls.pbAddMeasurementPoint->setEnabled(true);
}

void PlanningView::OnLoadPointPlanning()
{
  mMeasurementPointsNode = mSelectedNode;
  mMeasurementPlanningPointset = dynamic_cast<mitk::PointSet*>(mMeasurementPointsNode->GetData());

  LabeledPointSetMapper3D::Pointer mapper = LabeledPointSetMapper3D::New();
  mMeasurementPointsNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, mapper);

  LabeledPointSetMapper2D::Pointer mapper2D = LabeledPointSetMapper2D::New();
  mMeasurementPointsNode->SetStringProperty("default label","P");
  mMeasurementPointsNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard2D, mapper2D);

  // fill point group stack
  mMeasurementPointGroupStack = PointGroupStack(mMeasurementPlanningPointset);

  // add widgets and build connections
  for (int i=0; i<mMeasurementPointGroupStack.size(); i++)
  {
    auto group = mMeasurementPointGroupStack[i];
    mControls.vlPoints->addWidget(group->window);

    connect(group->pbSet, SIGNAL(clicked()), this, SLOT(OnSetMeasurementPoint()));
    connect(group->pbShow, SIGNAL(clicked()), this, SLOT(OnShowMeasurementPoint()));
    connect(group->pbRemove, SIGNAL(clicked()), this, SLOT(OnRemoveMeasurementPoint()));
  }

  mControls.pbAddMeasurementPoint->setEnabled(true);
}


void PlanningView::OnAddMeasurementPoint()
{
  PointGroup* group = mMeasurementPointGroupStack.pushNewPoint();

  // add widgets
  mControls.vlPoints->addWidget(group->window);

  // set point connection
  connect(group->pbSet, SIGNAL(clicked()), this, SLOT(OnSetMeasurementPoint()));

  // show point connection
  connect(group->pbShow, SIGNAL(clicked()), this, SIGNAL(OnShowMeasurementPoint()));

  // remove point connection
  connect(group->pbRemove, SIGNAL(clicked()), this, SIGNAL(OnRemoveMeasurementPoint()));

  // add initial point
  group->pbSet->clicked();
}

void PlanningView::OnSetMeasurementPoint()
{
  int pointNumber = mMeasurementPointGroupStack.findPointGroupFromSender(this->sender(),PointGroupStack::SetPoint);

  cout << "Set point " << pointNumber << std::endl;

  int pointId = pointNumber-1;
  if (pointId < 0)
    return;

  auto point = mInteractor->GetPosition();

  mMeasurementPlanningPointset->InsertPoint(pointId,point);
  mMeasurementPointGroupStack[pointId]->pbRemove->setEnabled(true);
  mMeasurementPointGroupStack[pointId]->pbShow->setEnabled(true);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void PlanningView::OnShowMeasurementPoint()
{
  int pointNumber = mMeasurementPointGroupStack.findPointGroupFromSender(this->sender(),PointGroupStack::ShowPoint);

  cout << "Show point " << pointNumber << std::endl;

  int pointId = pointNumber-1;
  if (pointId < 0)
    return;

  auto point = mMeasurementPlanningPointset->GetPoint(pointId);
  mInteractor->SetPosition(point);
  GetRenderWindowPart()->SetSelectedPosition(point);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void PlanningView::OnRemoveMeasurementPoint()
{
  int pointNumber = mMeasurementPointGroupStack.findPointGroupFromSender(this->sender(),PointGroupStack::RemovePoint);

  cout << "Remove point " << pointNumber << std::endl;

  int pointId = pointNumber-1;
  if (pointId < 0)
    return;

  mMeasurementPointGroupStack[pointId]->pbRemove->setEnabled(false);
  mMeasurementPointGroupStack[pointId]->pbShow->setEnabled(false);

  mMeasurementPlanningPointset->RemovePointIfExists(pointId);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
