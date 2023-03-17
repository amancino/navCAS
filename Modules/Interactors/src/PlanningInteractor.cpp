/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

// MITK
#include <mitkInteractionPositionEvent.h>

#include "AxisMapper3D.h"
#include "PlanningInteractor.h"

using namespace mitk;

PlanningInteractor::PlanningInteractor(const mitk::DataStorage::Pointer ds) :
  mDataStorage(ds)
{
  DrawAxis();
}

PlanningInteractor::~PlanningInteractor()
{
	cout << "Sphere interactor destructor" << std::endl;
  if (mDataStorage.IsNull())
    return;

  for (unsigned int i=0; i<mArrowContainer.size(); i++)
    mDataStorage->Remove(mArrowContainer[i]);
}

void PlanningInteractor::ConnectActionsAndFunctions()
{
  CONNECT_CONDITION("mouseClick", MouseClick);
  CONNECT_CONDITION("startCameraRotation", StartCameraRotation);
  CONNECT_CONDITION("cameraRotationFinished", CameraRotationFinished);
}

void PlanningInteractor::DrawAxis()
{
  double axis[3][3] = {{1.0, 0.0, 0.0},{0.0, 1.0, 0.0},{0.0, 0.0, 1.0}};

  for (int i=0; i<mArrowContainer.size(); i++)
  {
    if (mDataStorage->Exists(mArrowContainer[i]))
      mDataStorage->Remove(mArrowContainer[i]);
  }

  mArrowContainer.clear();
  std::string name[3] = {"X","Y","Z"};
  for (unsigned int i=0; i<3; i++)
  {
    mitk::DataNode::Pointer arrowNode = mitk::DataNode::New();

    // Set axis mapper
    mitk::PointSet::Pointer arrow = mitk::PointSet::New();
    mitk::Point3D center(0.0);
    mitk::Vector3D dir;
    dir[0]=axis[i][0];dir[1]=axis[i][1];dir[2]=axis[i][2];
    mitk::Point3D tip = center+dir;

    arrow->InsertPoint(center);
    arrow->InsertPoint(tip);
    arrowNode->SetData(arrow);

    AxisMapper3D::Pointer mapper = AxisMapper3D::New();
    arrowNode->SetMapper(mitk::BaseRenderer::StandardMapperSlot::Standard3D, mapper);
    arrowNode->SetColor(axis[i][0],axis[i][1],axis[i][2]);
    arrowNode->SetVisibility(true);
    arrowNode->SetProperty("navcas.display.dir",mitk::Vector3DProperty::New(dir));
    arrowNode->SetBoolProperty("helper object",true);
    arrowNode->SetBoolProperty("pickable",false);

    arrowNode->SetName(name[i].c_str());

    mDataStorage->Add(arrowNode);

    mArrowContainer.push_back(arrowNode);
  }
}

void PlanningInteractor::SetPosition(mitk::Point3D center)
{
  if (mArrowContainer.empty())
    return;

  for (int i=0; i<3; i++)
  {
    mitk::Vector3D dir;
    mArrowContainer[i]->GetPropertyValue("navcas.display.dir",dir);
    mitk::PointSet* pointSet = static_cast<mitk::PointSet*>(mArrowContainer[i]->GetData());
    pointSet->SetPoint(0,center);
    pointSet->SetPoint(1,center+60.0*dir);
  }

  mPosition = center;

}


void PlanningInteractor::SetDataNode(DataNode *node)
{
  DataInteractor::SetDataNode(node);

  // in case project was closed
  DrawAxis();
}




bool PlanningInteractor::MouseClick(const InteractionEvent* )
{
  mIsRotatingCamera=false;

  return false;
}

bool PlanningInteractor::StartCameraRotation(const InteractionEvent* )
{
  mIsRotatingCamera=true;
  return false;
}

bool PlanningInteractor::CameraRotationFinished(const InteractionEvent* InteractionEvent)
{
  const InteractionPositionEvent* interactionEvent = dynamic_cast<const InteractionPositionEvent*>(InteractionEvent);
  if (interactionEvent == nullptr)
    return false;

  if (!mIsRotatingCamera || (interactionEvent->GetSender()->GetName() != std::string("stdmulti.widget3")))
  {  
    mitk::Point3D position = interactionEvent->GetPositionInWorld();
    SetPosition(position);
    emit MoveCrossHair(position);
  }

  return false;
}

void PlanningInteractor::ShowArrow(bool show)
{
  for (unsigned int i=0; i<mArrowContainer.size(); i++)
    mArrowContainer[i]->SetVisibility(show);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
