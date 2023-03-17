/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>
#include <iomanip>

// vtk
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkImageMedian3D.h>
#include <vtkImageResample.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkFlyingEdges3D.h>
#include <vtkDecimatePro.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkTransformPolyDataFilter.h>

// itk
#include <itkBinaryThresholdImageFilter.h>

// mitk
#include <mitkImageCast.h>

#include "SurfaceFilter.h"



SurfaceFilter::SurfaceFilter()
{
  mOutputSurface = mitk::Surface::New();
  mThreshold = 100;
}

SurfaceFilter::~SurfaceFilter()
{

}

void SurfaceFilter::Update()
{

  // Cast Mitk Image to Itk
  typedef itk::Image<short,3> ImageType;
  ImageType::Pointer input = ImageType::New();
  mitk::CastToItkImage(mInputImage,input);

  // Apply threshold
  typedef itk::BinaryThresholdImageFilter<ImageType, ImageType> BinaryThresholdImageFilter;
  BinaryThresholdImageFilter::Pointer BinaryFilter = BinaryThresholdImageFilter::New();
  BinaryFilter->SetInput(input);
  BinaryFilter->SetLowerThreshold(mThreshold); // 75
  BinaryFilter->SetUpperThreshold(32000); // 100
  BinaryFilter->SetInsideValue(1);
  BinaryFilter->SetOutsideValue(0);
  BinaryFilter->Update();

  // Cast back from Itk Image to Mitk Image
  mitk::Image::Pointer mitkBinaryImage = mitk::Image::New();
  mitk::CastToMitkImage(BinaryFilter->GetOutput(),mitkBinaryImage);
  vtkSmartPointer<vtkImageData> vtkBinaryImage = mitkBinaryImage->GetVtkImageData();

  // Delete noise using median filter
  vtkSmartPointer<vtkImageMedian3D> median = vtkSmartPointer<vtkImageMedian3D>::New();
  median->SetInputData(vtkBinaryImage);
  median->SetKernelSize(3,3,3);
  median->ReleaseDataFlagOn();
  median->UpdateInformation();
  median->Update();

  //Interpolate image spacing
  vtkSmartPointer<vtkImageResample> imageresample = vtkSmartPointer<vtkImageResample>::New();
  imageresample->SetInputData(median->GetOutput());

  //Set Spacing Manual to 1mm in each direction (Original spacing is lost during image processing)
  imageresample->SetAxisOutputSpacing(0, 1.0);
  imageresample->SetAxisOutputSpacing(1, 1.0);
  imageresample->SetAxisOutputSpacing(2, 1.0);

  //imageresample->SetOutputOrigin(mInputImage->GetGeometry()->GetOrigin().GetDataPointer());

  imageresample->UpdateInformation();
  imageresample->Update();

  // Smooth surface using gaussian filter
  vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmooth = vtkSmartPointer<vtkImageGaussianSmooth>::New();
  gaussianSmooth->SetInputData(imageresample->GetOutput());
  gaussianSmooth->SetStandardDeviation(1.5);
  gaussianSmooth->SetDimensionality(3);
  gaussianSmooth->SetRadiusFactor(0.49);
  gaussianSmooth->ReleaseDataFlagOn();//
  gaussianSmooth->UpdateInformation();
  gaussianSmooth->Update();

  // Marching cubes: Image --> Surface
  vtkSmartPointer< vtkFlyingEdges3D> marchingCubes = vtkSmartPointer< vtkFlyingEdges3D>::New();
  marchingCubes->SetInputData(gaussianSmooth->GetOutput());
  marchingCubes->ComputeNormalsOff();
  marchingCubes->ComputeGradientsOff();
  marchingCubes->SetNumberOfContours(1);
  marchingCubes->SetValue(0,0.5);
  marchingCubes->Update();

  // Decimate if necesary
  vtkSmartPointer<vtkPolyData> skin = marchingCubes->GetOutput();
  vtkIdType cells = marchingCubes->GetOutput()->GetNumberOfCells();
  vtkIdType MAX_CELLS = 600000;
  if (cells > MAX_CELLS)
  {
    vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
    decimate->SplittingOff();
    decimate->SetErrorIsAbsolute(0);
    decimate->SetFeatureAngle(10);
    decimate->PreserveTopologyOn();
    decimate->BoundaryVertexDeletionOff();
    decimate->SetDegree(50); //std-value is 25!
    decimate->SetInputData(marchingCubes->GetOutput());
    decimate->SetTargetReduction((cells-MAX_CELLS)*1.0/cells);
    decimate->SetMaximumError(0.0001);
    decimate->Update();
    skin = decimate->GetOutput();
    cout << "Decimation took place, from " << marchingCubes->GetOutput()->GetNumberOfCells()
         << " cells to " << skin->GetNumberOfCells() << " cells" << std::endl;
  }

  // Smooth brain surface
  vtkSmartPointer<vtkSmoothPolyDataFilter> smoother = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
  smoother->SetInputData(skin);
  smoother->SetNumberOfIterations(100);
  smoother->SetRelaxationFactor(0.1);
  smoother->SetFeatureAngle(10);
  smoother->FeatureEdgeSmoothingOff();
  smoother->BoundarySmoothingOff();
  smoother->SetConvergence(0);
  smoother->Update();

  // Transform polydata using image geometry
  mitk::Vector3D spacing(input->GetSpacing());

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->SetMatrix(mInputImage->GetGeometry()->GetVtkMatrix());
  transform->PostMultiply();
  transform->Scale(1/spacing[0],1/spacing[1],1/spacing[2]);
  transform->Update();

  double tX = mInputImage->GetGeometry()->GetOrigin()[0] - transform->GetMatrix()->GetElement(0,3);
  double tY = mInputImage->GetGeometry()->GetOrigin()[1] - transform->GetMatrix()->GetElement(1,3);
  double tZ = mInputImage->GetGeometry()->GetOrigin()[2] - transform->GetMatrix()->GetElement(2,3);

  cout << tX << " " << tY << " " << tZ << std::endl;

  transform->Translate(tX,tY,tZ);
  transform->Update();

  vtkSmartPointer<vtkTransformPolyDataFilter> geomFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  geomFilter->SetInputData(smoother->GetOutput());
  geomFilter->SetTransform(transform);
  geomFilter->Update();

  // Set output
  mOutputSurface->SetVtkPolyData(geomFilter->GetOutput());
  //mOutputSurface->SetGeometry(mInputImage->GetGeometry());
  //mitk::Vector3D unitarySpacing(1.0);
  //mOutputSurface->GetGeometry()->SetSpacing(unitarySpacing);

}

