/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

//Vtk
#include <vtkCleanPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkDecimatePro.h>

//Mitk
#include <mitkDataStorage.h>
#include <mitkSurfaceVtkMapper3D.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkRenderingManager.h>
#include <mitkNodePredicateProperty.h>

#include "SurfaceAdaptation.h"

SurfaceAdaptation::SurfaceAdaptation()
{

}

SurfaceAdaptation::~SurfaceAdaptation()
{

}

const std::string SurfaceAdaptation::name = std::string("SurfaceAdaptationLowResolutionNode");

void SurfaceAdaptation::DeleteLowResolutionSurfaceFromParent(const mitk::DataNode* parent, mitk::DataStorage::Pointer ds)
{
  mitk::DataNode* node = ds->GetNamedDerivedNode(name.c_str(),parent);
  if (node != nullptr)
    ds->Remove(node);
}

mitk::Surface::Pointer SurfaceAdaptation::DecimateSurface(mitk::Surface::Pointer surf, double factor, bool forceExtremeDecimation)
{
  // Clean Poly Data
  vtkSmartPointer<vtkCleanPolyData> clean = vtkSmartPointer<vtkCleanPolyData>::New();
  clean->SetInputData(surf->GetVtkPolyData());
  clean->SetAbsoluteTolerance(0.01);
  clean->Update();

  std::cout << "Number of cells after clean: " << clean->GetOutput()->GetNumberOfCells() << std::endl;
  std::cout << "Number of points after clean: " << clean->GetOutput()->GetNumberOfPoints() << std::endl;

  // Triangulate
  vtkSmartPointer<vtkTriangleFilter> triagFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  triagFilter->SetInputData(clean->GetOutput());
  triagFilter->Update();

  vtkSmartPointer<vtkPolyData> input = vtkSmartPointer<vtkPolyData>::New();
  input = triagFilter->GetOutput();

  std::cout << "Starting surface smoothing" << std::endl;

  // Smooth surface
  vtkSmartPointer<vtkSmoothPolyDataFilter> smoother = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
  smoother->SetInputData(input);
  smoother->SetNumberOfIterations(40);
  smoother->SetRelaxationFactor(0.1);
  smoother->SetFeatureAngle(90);
  smoother->FeatureEdgeSmoothingOn();
  smoother->BoundarySmoothingOn();
  smoother->SetConvergence(0);
  smoother->Update();

  std::cout << "Starting " << factor << " factor decimation" << std::endl;

  // Decimate polydata
  vtkSmartPointer<vtkDecimatePro> decimator = vtkSmartPointer<vtkDecimatePro>::New();
  decimator->SetInputData(smoother->GetOutput());
  decimator->SetFeatureAngle(90);
  //decimator->SetSplitAngle(1);

  if (forceExtremeDecimation)
  {
    decimator->SetMaximumError(0.01);
    decimator->PreserveTopologyOn();
    decimator->SplittingOff();
  }
  else
  {
    decimator->SetMaximumError(0.0001);  // 0.5% error
    decimator->PreserveTopologyOn();
    decimator->SplittingOn(); // should be off, needs to be tested
  }

  decimator->SetTargetReduction(1.0-factor);   // 0.1 -> 10% reduction
  decimator->Update();

  std::cout << "Number of triangles (after decimation): " << decimator->GetOutput()->GetNumberOfCells() << std::endl;

  // Return decimated surface
  mitk::Surface::Pointer newSurf = mitk::Surface::New();
  newSurf->SetVtkPolyData(decimator->GetOutput());
  return newSurf;
}

bool SurfaceAdaptation::AttachLowResolutionSurface(mitk::DataStorage::Pointer ds, mitk::DataNode::Pointer parent)
{
  mitk::Surface::Pointer surf = dynamic_cast<mitk::Surface*>(parent->GetData());
  if (surf.IsNull())
    return false;

  long cells = surf->GetVtkPolyData()->GetNumberOfCells();


  bool needsDecimation = true;
  const long WISHED_CELLS = 20e3;
  if (cells < (WISHED_CELLS*1.05))
    needsDecimation=false;

  if ( ds->GetNamedDerivedNode(name.c_str(),parent) != nullptr)
    return false;

  // If node is already a low resolution node
  if (parent->GetName() == name)
    return false;

  double factor = (1.0*WISHED_CELLS)/cells;

  mitk::Surface::Pointer newSurf;
  if (needsDecimation)
    newSurf = SurfaceAdaptation::DecimateSurface(surf,factor);
  else
    newSurf = surf;

  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData(newSurf);
  newNode->SetName(name);
  newNode->SetVisibility(false);
  newNode->SetBoolProperty("helper object",true);
  newNode->SetBoolProperty("navCAS.planning.lowResolution",mitk::BoolProperty::New(true));
  ds->Add(newNode,parent);

  return true;
}

void SurfaceAdaptation::AttachLowResolutionSurfaces(mitk::DataStorage::Pointer ds)
{
  mitk::DataStorage::SetOfObjects::ConstPointer rs = ds->GetAll();

  // Iterate all nodes
  for(mitk::DataStorage::SetOfObjects::ConstIterator it = rs->Begin(); it != rs->End(); ++it)
    AttachLowResolutionSurface(ds,it->Value());
}

void SurfaceAdaptationThread::run()
{
  emit percentageCompleted(0);
  mRequiredAdaptations = SurfaceAdaptation::AdaptationsRequired(mDataStorage);

  if (mRequiredAdaptations == 0)
  {
    emit percentageCompleted(100);
    return;
  }

  emit percentageCompleted(3);

  if (mRequiredAdaptations == 1)
    emit percentageCompleted(50);

  int counter = 0;

  mitk::DataStorage::SetOfObjects::ConstPointer rs = mDataStorage->GetAll();

  // Iterate all nodes
  for(mitk::DataStorage::SetOfObjects::ConstIterator it = rs->Begin(); it != rs->End(); ++it)
  {
    mitk::Surface* surf = dynamic_cast<mitk::Surface*>(it->Value()->GetData());
    if (surf == nullptr)
    {
      std::cout << "Node is not a surface!" << std::endl;
      continue;
    }

    std::cout << it->Value()->GetName() << std::endl;
    long cells = surf->GetVtkPolyData()->GetNumberOfCells();

    const long WISHED_CELLS = 20e3;
    if (cells < (WISHED_CELLS*1.05))
      continue;

    if ( mDataStorage->GetNamedDerivedNode(SurfaceAdaptation::name.c_str(),it->Value()) != nullptr)
      continue;

    if (it->Value()->GetName() == SurfaceAdaptation::name)
      continue;

    if (mCancel)
    {
      emit cancellationFinished();
      return;
    }

    double factor = (1.0*WISHED_CELLS)/cells;

    mitk::Surface::Pointer newSurf = SurfaceAdaptation::DecimateSurface(surf,factor);

    emit percentageCompleted(counter*100/mRequiredAdaptations+5);

    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    newNode->SetData(newSurf);
    newNode->SetName(SurfaceAdaptation::name);
    newNode->SetVisibility(false);
    newNode->SetBoolProperty("helper object",true);
    newNode->SetBoolProperty("navCAS.planning.lowResolution",mitk::BoolProperty::New(true));


    //mDataStorage->Add(newNode,it->Value()); // Do not modify datastorage inside thread
    mLowResolutionStack.push_back(newNode);
    mParentStack.push_back(it->Value());

    emit percentageCompleted(++counter*100/mRequiredAdaptations-1);
  }

  emit percentageCompleted(100);
}

void SurfaceAdaptationThread::ResetStacks()
{
  unsigned int size = mParentStack.size();

  for (unsigned int i=0; i<size; i++)
  {
    mParentStack.pop_back();
    mLowResolutionStack.pop_back();
  }
}

unsigned int SurfaceAdaptation::AdaptationsRequired(mitk::DataStorage::Pointer ds)
{
  unsigned int counter = 0;
  mitk::DataStorage::SetOfObjects::ConstPointer rs = ds->GetAll();

  // Iterate all nodes
  for(mitk::DataStorage::SetOfObjects::ConstIterator it = rs->Begin(); it != rs->End(); ++it)
  {
    mitk::Surface* surf = dynamic_cast<mitk::Surface*>(it->Value()->GetData());
    if (surf == nullptr)
		{
			continue;
		}

    if (surf->GetVtkPolyData() == nullptr)
		{
			continue;
		}

    long cells = surf->GetVtkPolyData()->GetNumberOfCells();

		std::cout << "cells: " << cells << std::endl;
    const long WISHED_CELLS = 20e3;
    if (cells < (WISHED_CELLS*1.05))
		{
			continue;
		}

    if (ds->GetNamedDerivedNode(SurfaceAdaptation::name.c_str(), it->Value()) != nullptr)
		{
			continue;
		}

		if (it->Value()->GetName() == SurfaceAdaptation::name)
		{
			continue;
		}

    counter++;
		std::cout << "counter: " << counter << std::endl;
  }

  return counter;
}

unsigned int SurfaceAdaptation::RemoveOrphanNodes(mitk::DataStorage::Pointer ds)
{
  unsigned int counter = 0;
  mitk::DataStorage::SetOfObjects::ConstPointer nodes = ds->GetSubset(mitk::NodePredicateProperty::New("navCAS.planning.lowResolution",mitk::BoolProperty::New(true)));

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = nodes->Begin(); it != nodes->End(); ++it)
  {
    // Check and remove posible orphan low resolution nodes
    mitk::DataStorage::SetOfObjects::ConstPointer sources = ds->GetSources(it->Value());
    if (sources->empty())
    {
      ds->Remove(it->Value());
      counter++;
    }
  }
  return counter;
}

mitk::DataNode::Pointer SurfaceAdaptation::GetLowResolutionNode(mitk::DataStorage::Pointer ds, mitk::DataNode::Pointer parent)
{
  return ds->GetNamedDerivedNode(SurfaceAdaptation::name.c_str(),parent);
}
