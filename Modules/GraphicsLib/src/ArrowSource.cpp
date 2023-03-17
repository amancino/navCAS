#include <vtkArrowSource.h>
#include <vtkMath.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransformFilter.h>
#include <vtkLineSource.h>

#include "ArrowSource.h"


ArrowSource::ArrowSource(double *normal, double *center, double length, double radius)
{
  vtkSmartPointer<vtkArrowSource> arrow = vtkSmartPointer<vtkArrowSource>::New();
  arrow->SetTipResolution(17);
  arrow->SetShaftResolution(17);
  arrow->SetShaftRadius(radius);
  arrow->SetTipRadius(radius);
  arrow->SetTipLength(0.125);
  arrow->Update();

  vtkSmartPointer<vtkTransform> scaleTransform = vtkSmartPointer<vtkTransform>::New();
  scaleTransform->Scale(length,1.0,1.0);

  vtkSmartPointer<vtkTransformPolyDataFilter> scaleTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  scaleTransformFilter->SetInputData(arrow->GetOutput());
  scaleTransformFilter->SetTransform(scaleTransform);
  scaleTransformFilter->Update();

  mOriginalArrow = scaleTransformFilter->GetOutput();

  SetNormal(normal);
  SetCenter(center);
  Update();
}


ArrowSource::~ArrowSource()
{
  cout << "ArrowSource destructor" << std::endl;
}

void ArrowSource::SetNormal(double* normal)
{
  for (unsigned int i=0; i<3; i++)
    mNormal[i] = normal[i];
}

void ArrowSource::SetCenter(double* center)
{
  for (unsigned int i=0; i<3; i++)
    mCenter[i] = center[i];
}

void ArrowSource::Update()
{
  // ** Rotation 1 (Rotation of the Normal vector) **
  // Normal angle determination, after moving plane widget
  double pn[3] = {1.0, 0.0, 0.0};
  double result[3];
  double angle;

  vtkMath::Cross(pn,mNormal,result);
  angle = atan2(vtkMath::Norm(result),vtkMath::Dot(pn,mNormal));
  angle = vtkMath::DegreesFromRadians(angle);

  // Rotation 1
  vtkSmartPointer<vtkTransform> rotation = vtkSmartPointer<vtkTransform>::New();
  rotation->RotateWXYZ(angle,result);
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(mOriginalArrow);
  transformFilter->SetTransform(rotation);
  transformFilter->Update();

  // Translation to point
  vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
  translation->Translate(mCenter);
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter2 = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter2->SetInputData(transformFilter->GetOutput());
  transformFilter2->SetTransform(translation);
  transformFilter2->Update();

  mOutput = transformFilter2->GetOutput();
}

vtkSmartPointer<vtkPolyData> ArrowSource::GetOutput()
{
  return mOutput;
}
