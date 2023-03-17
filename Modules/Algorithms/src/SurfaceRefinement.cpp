/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>

#include <itkEuclideanDistancePointMetric.h>
#include <itkEuler3DTransform.h>
#include <itkLevenbergMarquardtOptimizer.h>
#include <itkPointSetToPointSetRegistrationMethod.h>
#include <itkRigid3DTransform.h>

#include <vtkPolyData.h>
#include <vtkCleanPolyData.h>

#include <mitkSurface.h>
#include <mitkNodePredicateProperty.h>

#include "SurfaceRefinement.h"

using namespace std;



vtkSmartPointer<vtkMatrix4x4> SurfaceRefinementThread::GetVtkRegistrationMatrix(itk::Rigid3DTransform<double>::Pointer transform, bool verbose)
{
  // Build 4x4 matrix
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  for (int i=0; i<3; i++)
  {
    for (int j=0; j<3; j++)
      matrix->SetElement(i,j,transform->GetMatrix()[i][j]);//transposed matrix is (j,i)

    matrix->SetElement(i,3,transform->GetTranslation()[i]);
  }
  matrix->SetElement(3,0,0.0);
  matrix->SetElement(3,1,0.0);
  matrix->SetElement(3,2,0.0);
  matrix->SetElement(3,3,1.0);

  /*
  if (verbose)
  {
    cout << "Before invert: " << std::endl;
    for (int i=0; i<4; i++)
    {
      for (int j =0; j<4; j++)
        cout << matrix->GetElement(i,j) << " ";

      cout << std::endl;
    }
  }
  */

  matrix->Invert();

  /*
  if (verbose)
  {
    cout << "After invert: " << std::endl;
    for (int i=0; i<4; i++)
    {
      for (int j =0; j<4; j++)
        cout << matrix->GetElement(i,j) << " ";

      cout << std::endl;
    }
  }
  */

  double center[3];
  for (unsigned int i=0; i<3; i++)
    center[i] = transform->GetCenter()[i];

  vtkSmartPointer<vtkTransform> registrationTransform = vtkSmartPointer<vtkTransform>::New();
  registrationTransform->Identity();
  registrationTransform->Translate(-center[0],-center[1],-center[2]);
  registrationTransform->PostMultiply();
  registrationTransform->Concatenate(matrix);
  registrationTransform->Translate(center);
  registrationTransform->Update();

  double wxyz[4];
  registrationTransform->GetOrientationWXYZ(wxyz);
  double axis[3];axis[0]=wxyz[1]; axis[1]=wxyz[2]; axis[2]=wxyz[3];

  double translation[3];
  translation[0] = matrix->GetElement(0,3);
  translation[1] = matrix->GetElement(1,3);
  translation[2] = matrix->GetElement(2,3);

  if (verbose)
  {
    cout << "Angle: " << wxyz[0] << std::endl;
    cout << "Versor: " << mitk::Vector3D(axis) << std::endl;
    cout << "Translation: " << mitk::Vector3D(transform->GetTranslation()) << std::endl;
    cout << "Translation after invert: " << mitk::Vector3D(translation) << std::endl;
  }

  return registrationTransform->GetMatrix();
}

void SurfaceRefinementThread::run()
{
  cout << "Starting surface refinement" << std::endl;

  const unsigned int Dimension = 3;

  // First, define the necessary types for the moving and fixed point sets.
  typedef itk::PointSet< double, Dimension > PointSetType;
  PointSetType::Pointer fixedPointSet  = PointSetType::New();
  PointSetType::Pointer movingPointSet = PointSetType::New();
  using PointType = PointSetType::PointType;
  using PointsContainer = PointSetType::PointsContainer;
  PointsContainer::Pointer fixedPointContainer  = PointsContainer::New();
  PointsContainer::Pointer movingPointContainer = PointsContainer::New();
  PointType fixedPoint;
  PointType movingPoint;



  // Update the fixed points.
  vtkPolyData* pd = dynamic_cast<mitk::Surface*>(mSurfaceNode->GetData())->GetVtkPolyData();

  vtkSmartPointer<vtkCleanPolyData> cleanPolyData = vtkSmartPointer<vtkCleanPolyData>::New();
  cleanPolyData->SetInputData(pd);
  cleanPolyData->SetTolerance(0.001);
  cleanPolyData->Update();

  cout << "Number of points of original surface: " << pd->GetNumberOfPoints() << std::endl;
  cout << "Number of points after clean: " << cleanPolyData->GetOutput()->GetNumberOfPoints() << std::endl;


  vtkPoints* pdPoints = cleanPolyData->GetOutput()->GetPoints();

  unsigned int pointId = 0;
  for (unsigned int n=0; n<=cleanPolyData->GetOutput()->GetNumberOfPoints(); n++)
  {
    fixedPoint = pdPoints->GetPoint(n);
    fixedPointContainer->InsertElement( pointId, fixedPoint );
    pointId++;
  }

  fixedPointSet->SetPoints( fixedPointContainer );
  std::cout <<
    "Number of fixed Points = " << fixedPointSet->GetNumberOfPoints()
    << std::endl;

  // Update the moving points.
  pointId = 0;
  for (int n=0; n<mPointSet->GetSize(); n++)
  {
    mitk::Point3D point = mPointSet->GetPoint(n);

    movingPoint = point.GetDataPointer();
    movingPointContainer->InsertElement( pointId, movingPoint );
    pointId++;
  }

  movingPointSet->SetPoints( movingPointContainer );
  std::cout <<
    "Number of moving Points = " << movingPointSet->GetNumberOfPoints()
    << std::endl;


// After the points are read, setup the metric to be used
// later by the registration.

  typedef itk::EuclideanDistancePointMetric<PointSetType, PointSetType> MetricType;

  MetricType::Pointer metric = MetricType::New();



// Next, setup the tranform, optimizers, and registration.
  using TransformType = itk::Euler3DTransform< double >;
  //using TransformType = itk::VersorRigid3DTransform< double >; // worse results!
  TransformType::Pointer transform = TransformType::New();
  // Optimizer Type
  using OptimizerType = itk::LevenbergMarquardtOptimizer;
  OptimizerType::Pointer      optimizer     = OptimizerType::New();
  optimizer->SetUseCostFunctionGradient(false);

  // Registration Method
  using RegistrationType = itk::PointSetToPointSetRegistrationMethod<
                            PointSetType, PointSetType >;
  RegistrationType::Pointer   registration  = RegistrationType::New();
// Scale the translation components of the Transform in the Optimizer

  OptimizerType::ScalesType scales( transform->GetNumberOfParameters() );

// Next, set the scales and ranges for translations and rotations in the
// transform. Also, set the convergence criteria and number of iterations
// to be used by the optimizer.
  constexpr double translationScale = 0.001; // dynamic range of translations
  constexpr double rotationScale = 0.001;       // dynamic range of rotations
  scales[0] = 1.0 / rotationScale;
  scales[1] = 1.0 / rotationScale;
  scales[2] = 1.0 / rotationScale;
  scales[3] = 1.0 / translationScale;
  scales[4] = 1.0 / translationScale;
  scales[5] = 1.0 / translationScale;
  optimizer->SetScales( scales );
  mMaxIterations = 1000;
  optimizer->SetNumberOfIterations( mMaxIterations );
  optimizer->SetValueTolerance( 1e-11 );
  optimizer->SetGradientTolerance( 1e-11 );
  optimizer->SetEpsilonFunction( 1e-11 );

// Here we start with an identity transform, although the user will usually
// be able to provide a better guess than this.
  transform->SetIdentity();
  registration->SetInitialTransformParameters( transform->GetParameters() );

// Connect all the components required for the registration.
  registration->SetMetric(        metric        );
  registration->SetOptimizer(     optimizer     );
  registration->SetTransform(     transform     );
  registration->SetFixedPointSet( fixedPointSet );
  registration->SetMovingPointSet(   movingPointSet   );

  // Connect an observer
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );
  connect(observer,SIGNAL(newStep()),this,SLOT(OnNewStep()));

  mFilteringOutliers=false;
  mCurrentStep = 0;
  try
  {
    registration->Update();
  }
  catch( itk::ExceptionObject & e )
  {
    std::cerr << e << std::endl;
    return;
  }
  std::cout << "Solution = " << transform->GetParameters() << std::endl;
  std::cout << "Stopping condition: " << optimizer->GetStopConditionDescription() << std::endl;
  OptimizerType::MeasureType value = optimizer->GetValue();
  double meanError = value.mean();
  double stdError = value.rms();
  cout << "Final value: " << std::endl << value << std::endl;
  cout << "Mean error: " << meanError << std::endl;
  cout << "std deviation of points: " << stdError << std::endl;


  cout << "inverse matrix before excluding outliers: " << std::endl;
  vtkSmartPointer<vtkMatrix4x4> firstResult = GetVtkRegistrationMatrix(static_cast<itk::Rigid3DTransform<double>* >(transform));
  for (int i=0; i<4; i++)
  {
    for (int j =0; j<4; j++)
      cout << firstResult->GetElement(i,j) << " ";

    cout << std::endl;
  }


  // find outliers
  cout << "Outliers: " << std::endl;
  std::vector<unsigned int> outliers;
  for (unsigned int i=0; i<value.GetSize(); i++)
  {
    if ( value[i] > 3*stdError )
    {
      cout << "Point " << i << ": " << value[i] << std::endl;
      outliers.push_back(i);
    }
  }

  // If outliers are identified, exclude them
  if (outliers.size()>0)
  {
    PointsContainer::Pointer validPointsContainer = PointsContainer::New();
    unsigned int id =0;
    for (unsigned int p = 0; p < movingPointSet->GetNumberOfPoints(); p++)
    {
      bool outlier = false;
      for (unsigned int i=0; i<outliers.size(); i++)
      {
        if (p == outliers[i])
        {
          outlier = true;
          break;
        }
      }
      if (!outlier)
      {
        validPointsContainer->InsertElement(id,movingPointContainer->GetElement(p));
        id++;
      }
    }

    movingPointSet->SetPoints(validPointsContainer);
    cout << "Number of valid points: " << movingPointSet->GetNumberOfPoints() << std::endl;
    registration->SetMovingPointSet( movingPointSet );
  }

  // second registration (TO DO: start with already transformed points)
  mMaxIterations = 1000;
  optimizer->SetNumberOfIterations( mMaxIterations );
  optimizer->SetValueTolerance( 1e-14 );
  optimizer->SetGradientTolerance( 1e-14 );
  optimizer->SetEpsilonFunction( 1e-14 );
  mFilteringOutliers=true;
  mCurrentStep = 0;

  try
  {
    registration->Update();
  }
  catch( itk::ExceptionObject & e )
  {
    std::cerr << e << std::endl;
    return;
  }
  std::cout << "Solution = " << transform->GetParameters() << std::endl;
  std::cout << "Stopping condition: " << optimizer->GetStopConditionDescription() << std::endl;

  // Use the direct transformation matrix
  vtkSmartPointer<vtkMatrix4x4> inverseTansform = GetVtkRegistrationMatrix(static_cast<itk::Rigid3DTransform<double>* >(transform));

  // display the transformed secondary pointset according to matrix

  // Transform points shown on screen
  vtkSmartPointer<vtkTransform> registrationTransform = vtkSmartPointer<vtkTransform>::New();
  registrationTransform->SetMatrix(inverseTansform);
  registrationTransform->Update();


  // moved pointset (inverting matrix)
  registrationTransform->Inverse();
  registrationTransform->Update();

  // store transformation as inverted matrix
  mSurfaceRefinementMatrix = registrationTransform->GetMatrix();

  cout << "inverse matrix: " << std::endl;
  for (int i=0; i<4; i++)
  {
    for (int j =0; j<4; j++)
      cout << inverseTansform->GetElement(i,j) << " ";

    cout << std::endl;
  }

  // red points are outliers
  mGreenPointset = mitk::PointSet::New();
  mRedPointset = mitk::PointSet::New();
  for (int i=0; i<mPointSet->GetSize(); i++)
  {
    double newPoint[3];
    registrationTransform->TransformPoint(mPointSet->GetPoint(i).GetDataPointer(),newPoint);

    if (value[i] < stdError)
      mGreenPointset->InsertPoint(i,mitk::Point3D(newPoint));
    else
      mRedPointset->InsertPoint(i,mitk::Point3D(newPoint));
  }

  mMeanError = optimizer->GetValue().mean();
  mRMSError = optimizer->GetValue().rms();
}

void SurfaceRefinementThread::OnNewStep()
{
  int percent = mFilteringOutliers? 50 : 0;
  percent += 50*(mCurrentStep++)/mMaxIterations;
  emit percentageCompleted(percent);
}


