/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

//Vtk
#include <vtkCamera.h>

//Itk

#include <QmitkNodeDescriptorManager.h>

//Mitk
#include <mitkDataStorage.h>
#include <mitkNodePredicateProperty.h>
#include <mitkTimeGeometry.h>
#include <mitkNodePredicateAnd.h>
#include <mitkImage.h>

#include "ViewCommands.h"

ViewCommands::ViewCommands()
{

}

ViewCommands::~ViewCommands()
{

}

void ViewCommands::TurnAllSurfacesPickable(const mitk::DataStorage* ds)
{
  // Turn every STL pickable
  mitk::DataStorage::SetOfObjects::ConstPointer nodes = ds->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = nodes->Begin(); it != nodes->End(); ++it)
  {
    mitk::DataNode::Pointer node = it->Value();

    mitk::Surface* surf = dynamic_cast<mitk::Surface*>(node->GetData());
    if (surf != nullptr)
    {
      node->SetBoolProperty("pickable",true);
    }
  }
}

void ViewCommands::GlobalReinit(const mitk::DataStorage* ds, mitk::NodePredicateProperty::Pointer pred)
{
  mitk::NodePredicateProperty::Pointer visible = mitk::NodePredicateProperty::New("visible",mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer predAnd = mitk::NodePredicateAnd::New();
  predAnd->AddPredicate(visible);

  if (pred.IsNotNull())
    predAnd->AddPredicate(pred);

  mitk::DataStorage::SetOfObjects::ConstPointer so = ds->GetSubset(predAnd);

  // calculate bounding geometry of these nodes
  mitk::TimeGeometry::ConstPointer bounds = ds->ComputeBoundingGeometry3D(so);

  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
}

void ViewCommands::Reinit(const mitk::DataNode::Pointer node, const mitk::DataStorage* ds)
{
  if (node.IsNull())
    return;

  mitk::DataStorage::SetOfObjects::Pointer so = mitk::DataStorage::SetOfObjects::New();
  so->InsertElement(0,node);

  auto image = dynamic_cast<mitk::Image*>(node->GetData());

  // image
  if (nullptr != image) // ... image is selected, reinit is expected to rectify askew images.
  {
    mitk::RenderingManager::GetInstance()->InitializeViews(image->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
  }

  // surface
  else
  {
    // calculate bounding geometry of these nodes
    mitk::TimeGeometry::ConstPointer bounds = ds->ComputeBoundingGeometry3D(so);

    // initialize the views to the bounding geometry
    mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
  }
}



void ViewCommands::SetParallelView(QmitkRenderWindow* qmitkRenderWindow)
{
  vtkSmartPointer<vtkCamera> activeCamera3D;
  activeCamera3D = qmitkRenderWindow->GetRenderer()->GetVtkRenderer()->GetActiveCamera();
  activeCamera3D->SetParallelProjection(1);
}

void ViewCommands::Set3DCrosshairVisibility(bool vis, mitk::DataStorage::Pointer ds, QmitkRenderWindow *threeD)
{
  auto plane1 = ds->GetNamedNode("stdmulti.widget0.plane");
  auto plane2 = ds->GetNamedNode("stdmulti.widget1.plane");
  auto plane3 = ds->GetNamedNode("stdmulti.widget2.plane");

  if (plane1)
    plane1->SetVisibility(vis,threeD->GetRenderer());
  if (plane2)
    plane2->SetVisibility(vis,threeD->GetRenderer());
  if (plane3)
    plane3->SetVisibility(vis,threeD->GetRenderer());

  if (!plane1 || !plane2 || !plane3)
  {
    cout << "No plane nodes found in datastorage!" << std::endl;

    cout << "Current present nodes are: " << std::endl;
    auto so = ds->GetAll();
    for (auto it=so->Begin(); it!=so->End();++it)
      cout << it->Value()->GetName() << std::endl;
  }

}

void ViewCommands::AddNodeDescriptor(QString property, QString name, QString icon)
{
  auto pred = mitk::NodePredicateProperty::New(property.toStdString().c_str());
  auto manager = QmitkNodeDescriptorManager::GetInstance();
  auto descriptor = new QmitkNodeDescriptor(name,icon,pred, manager);
  manager->AddDescriptor(descriptor);
}


void ViewCommands::RemoveNodeDescriptor(QString name)
{
  auto manager = QmitkNodeDescriptorManager::GetInstance();
  auto descriptor = manager->GetDescriptor(name);
  auto actionlist = descriptor->GetActions();
  for (auto action : actionlist)
  {
    if (action->text() == "Remove")
      action->setEnabled(false);
  }
  manager->RemoveDescriptor(descriptor);
}
