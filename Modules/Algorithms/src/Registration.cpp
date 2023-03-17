/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>

// itk
#include <itkVersorRigid3DTransform.h>
#include <itkLandmarkBasedTransformInitializer.h>
#include <itkRigid3DTransform.h>

// vtk
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkMath.h>
#include <vtkTransform.h>
#include <vtkPolyData.h>

#include "Registration.h"
#include "SurfaceRefinement.h"

using namespace std;

void Registration::GetErrorMetricsFromPairedPointRegistration(  const mitk::PointSet::Pointer plannedPoints,
                                                              const mitk::PointSet::Pointer transformedPoints,
                                                              std::vector<double> &distances,
                                                              double &meanError, double &std, double &fre, double &fle)
{
  const unsigned int numberOfPoints = plannedPoints->GetSize();

  distances.clear();
  meanError = 0.0;
  double fre2 = 0.0;
  for (unsigned int i=0; i<numberOfPoints; i++)
  {
    double dist2 = vtkMath::Distance2BetweenPoints(transformedPoints->GetPoint(i).GetDataPointer(),plannedPoints->GetPoint(i).GetDataPointer());
    double dist = sqrt(dist2);
    //cout << "Error in point " << i << ": " << dist << " mm" << std::endl;
    meanError += dist;
    fre2 += dist2;

    distances.push_back(dist);
  }

  meanError /= numberOfPoints;
  fre2 /= numberOfPoints;
  //cout << "Mean error: " << meanError << " mm" << std::endl;

  // compute std
  std = 0.0;
  for (unsigned int i=0; i<numberOfPoints; i++)
  {
    double dist = sqrt(vtkMath::Distance2BetweenPoints(transformedPoints->GetPoint(i).GetDataPointer(),plannedPoints->GetPoint(i).GetDataPointer()));
    std += pow(dist - meanError,2);
  }
  std /= numberOfPoints;
  std = sqrt(std);

  //cout << "std: " << std << " mm" << std::endl;

  //cout << "FRE^2: " << fre2 << " mm^2" << std::endl;
  fre = sqrt(fre2);
  //cout << "FRE (rms): " << fre << " mm" << std::endl;
  fle = fre/(1.0-1.0/(2.0*numberOfPoints));
  //cout << "Estimated FLE: " << fle << " mm" << std::endl;
}

vtkSmartPointer<vtkMatrix4x4> Registration::PerformPairedPointsRegistration(const mitk::PointSet::Pointer plannedPoints, const mitk::PointSet::Pointer realPoints, mitk::PointSet::Pointer transformedPoints, bool verbose)
{
  if (verbose)
  {
    cout << "Real number of valid points: " << realPoints->GetSize() << std::endl;
    cout << "Planned points size: " << plannedPoints->GetSize() << std::endl;
  }
  using Rigid3DTransformType = itk::VersorRigid3DTransform< double >;

  typedef itk::Image<double,3> ImageType;
  using LandmarkBasedTransformInitializerType = itk::LandmarkBasedTransformInitializer< Rigid3DTransformType, ImageType, ImageType >;

  LandmarkBasedTransformInitializerType::Pointer landmarkBasedTransformInitializer =
    LandmarkBasedTransformInitializerType::New();
  //  Create source and target landmarks.
  using LandmarkContainerType = LandmarkBasedTransformInitializerType::LandmarkPointContainer;
  using LandmarkPointType = LandmarkBasedTransformInitializerType::LandmarkPointType;
  LandmarkContainerType fixedLandmarks;
  LandmarkContainerType movingLandmarks;

  // Add paired points for registration
  auto pIt = plannedPoints->Begin();
  for (auto rIt = realPoints->Begin(); rIt != realPoints->End(); ++rIt, ++pIt)
  {
    mitk::Point3D rPoint = rIt->Value();
    mitk::Point3D pPoint = pIt->Value();

    LandmarkPointType fixedPoint = pPoint.GetDataPointer();
    LandmarkPointType movingPoint = rPoint.GetDataPointer();
    fixedLandmarks.push_back(fixedPoint);
    movingLandmarks.push_back(movingPoint);
  }

  // Set points
  landmarkBasedTransformInitializer->SetFixedLandmarks( fixedLandmarks );
  landmarkBasedTransformInitializer->SetMovingLandmarks( movingLandmarks );

  Rigid3DTransformType::Pointer transform = Rigid3DTransformType::New();
  transform->SetIdentity();

  landmarkBasedTransformInitializer->SetTransform(transform);
  //landmarkBasedTransformInitializer->InitializeTransform();
  landmarkBasedTransformInitializer->InitializeTransform();

  if (verbose)
  {
    MITK_INFO << "Considered points: " << fixedLandmarks.size();
    cout << "Center of rotation: " << transform->GetCenter() << std::endl;
    cout << "Offset: " << transform->GetOffset() << std::endl;
    cout << "Total translation: " << transform->GetTranslation() << std::endl;
    cout << "Rotation matrix: " << std::endl << transform->GetMatrix() << std::endl;
  }
  auto matrix = SurfaceRefinementThread::GetVtkRegistrationMatrix(static_cast<itk::Rigid3DTransform<double>*>(transform),verbose);

  if (transformedPoints.IsNotNull())
  {
    // Transform points
    vtkSmartPointer<vtkTransform> registrationTransform = vtkSmartPointer<vtkTransform>::New();
    registrationTransform->SetMatrix(matrix);
    registrationTransform->Update();

    transformedPoints->Clear();
    for (auto rIt = realPoints->Begin(); rIt != realPoints->End(); ++rIt)
    {
      mitk::Point3D rPoint = rIt->Value();
      transformedPoints->InsertPoint(rIt->Index(),mitk::Point3D(registrationTransform->TransformDoublePoint(rPoint.GetDataPointer())));
    }
  }

  return matrix;
}

void Registration::GetAngleAndOffsetErrorFromMatrix(const vtkMatrix4x4* matrix, double &offset, double &angle)
{
  double wxyz[4];
  double translation[3];

  vtkSmartPointer<vtkTransform> errorTransform = vtkSmartPointer<vtkTransform>::New();
  errorTransform->SetMatrix(const_cast<vtkMatrix4x4*>(matrix));
  errorTransform->Update();
  //cout << "Total error transformation: " << std::endl;

  errorTransform->GetOrientationWXYZ(wxyz);
  angle = abs(wxyz[0]);
  //cout << "Angle error: " << angle << " deg" << std::endl;

  errorTransform->GetPosition(translation);
  mitk::Vector3D offsetVec(translation);
  offset = offsetVec.GetNorm();

  if (angle > 180.0)
    angle = 360.0 - angle;
  //cout << "Offset error: " << offset << " mm" << std::endl;
}

double Registration::EstimateTREFromMatrix(const vtkMatrix4x4* matrix, vtkPolyData* pd, const int npoints)
{
  auto transform = vtkSmartPointer<vtkTransform>::New();
  transform->SetMatrix(const_cast<vtkMatrix4x4*>(matrix));
  transform->Update();

  //std::vector<mitk::Vector3D> dist;
  //dist.reserve(npoints);

  const int N = pd->GetNumberOfPoints();
  const int step = N/npoints;
  double sum = 0.0;
  for (int i=0; i<npoints; ++i)
  {
    // pick pd point
    const int id = i*step;
    mitk::Point3D pdPoint(pd->GetPoint(id));

    // transform using matrix
    mitk::Point3D transformedPoint(transform->TransformDoublePoint(pdPoint.GetDataPointer()));

    // store difference
    //dist.push_back(pdPoint-transformedPoint);

    // compute distance
    double dist2 = vtkMath::Distance2BetweenPoints(pdPoint.GetDataPointer(),transformedPoint.GetDataPointer());
    sum += dist2;
  }

  double mean = sqrt(sum / npoints);

  return mean;
}

