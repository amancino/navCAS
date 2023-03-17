/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include "RegistrationErrorVisualization.h"

#include <mitkDataNode.h>
#include <mitkSurface.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkVtkScalarModeProperty.h>

#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkLookupTable.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>


RegistrationErrorVisualization::RegistrationErrorVisualization()
{

}

// the algorithm assumes that the point ids were not modified during registration process
vtkSmartPointer<vtkLookupTable> RegistrationErrorVisualization::ColorizeNodes(mitk::DataNode* plannedNode, mitk::DataNode* movingNode,
                                                                              double &mean, double &std)
{
  // get fisrt surface (fixed)
  auto fixed = dynamic_cast<mitk::Surface*>(plannedNode->GetData())->GetVtkPolyData();

  // get second surface (moving)
  auto moving = dynamic_cast<mitk::Surface*>(movingNode->GetData())->GetVtkPolyData();

  // iterate through all cells of main surface and compute difference with the moving ones

  double sum = 0.0;

  // create cell data array
  double maxDist = 0.0;
  vtkSmartPointer<vtkFloatArray> cellData = vtkSmartPointer<vtkFloatArray>::New();
  for (auto id = 0; id < fixed->GetNumberOfCells(); id++)
  {
    vtkIdType pid = moving->GetCell(id)->GetPointId(0);

    double dist2 = vtkMath::Distance2BetweenPoints(fixed->GetPoint(pid),moving->GetPoint(pid));
    double dist = sqrt(dist2);

    sum += dist;

    cellData->InsertNextValue(dist);
    //cout << dist << "mm" << endl;

    maxDist = std::max(maxDist,dist);
  }
  // get mean
  mean = sum / fixed->GetNumberOfCells();

  // iterate again to compute std
  double var = 0.0;
  for (auto id = 0; id < fixed->GetNumberOfCells(); id++)
  {
    vtkIdType pid = moving->GetCell(id)->GetPointId(0);

    double dist2 = vtkMath::Distance2BetweenPoints(fixed->GetPoint(pid),moving->GetPoint(pid));
    double dist = sqrt(dist2);

    var += pow((dist - mean),2);
  }
  std = sqrt(var / (fixed->GetNumberOfCells()));

  moving->GetCellData()->SetScalars(cellData);

  // assign colorimetry
  // Start by creating a black/white lookup table.
   vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
   lut->SetTableRange (0, maxDist);
   lut->SetSaturationRange (1, 1);
   lut->SetHueRange (0.33, 0.0);
   lut->SetValueRange (1, 1);
   lut->Build(); //effective built

   mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
   mitkLut->SetVtkLookupTable(lut);

   movingNode->SetProperty("LookupTable", mitk::LookupTableProperty::New(mitkLut));
   movingNode->SetFloatProperty("ScalarsRangeMinimum", 0.0);
   movingNode->SetFloatProperty("ScalarsRangeMaximum", maxDist);
   movingNode->SetBoolProperty("scalar visibility", true);
   movingNode->SetBoolProperty("color mode", true);

   mitk::VtkScalarModeProperty::Pointer scalarMode = mitk::VtkScalarModeProperty::New();
   scalarMode->SetScalarModeToCellData();
   movingNode->SetProperty("scalar mode", scalarMode);
   movingNode->Update();

   // material properties
   movingNode->SetFloatProperty("material.specularCoefficient",0.0);
   plannedNode->SetVisibility(false);

  return lut;
}
