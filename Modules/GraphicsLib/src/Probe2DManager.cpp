/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <mitkRenderWindow.h>
#include <mitkRenderingManager.h>
#include <mitkBaseRenderer.h>
#include <mitkManualPlacementAnnotationRenderer.h>

#include <QmitkRenderWindow.h>

#include "Probe2DManager.h"

#include <vtkImageCanvasSource2D.h>
#include "Probe2D.h"

using namespace mitk;

Probe2DManager::Probe2DManager(QmitkRenderWindow* renderWindow) : mRenderWindow(renderWindow)
{
  if (renderWindow == nullptr)
  {
    MITK_ERROR << "Render window is null" << std::endl;
    return;
  }

  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(mRenderWindow->GetVtkRenderWindow());

  // Overwrites the existing overlay manager to create one manager per renderer (one for each window).
  //mitk::OverlayManager::Pointer OverlayManagerInstance = mitk::OverlayManager::New();
  //renderer->SetOverlayManager(OverlayManagerInstance);


  mTranslationOverlay = mitk::TextAnnotation2D::New();
  mTranslationOverlay->SetText("+"); //set UTF-8 encoded text to render
  mTranslationOverlay->SetFontSize(20);
  mTranslationOverlay->SetColor(0,1,1); //Set text color to white
  mTranslationOverlay->SetOpacity(1);
  mTranslationOverlay->SetVisibility(false);

  mitk::ManualPlacementAnnotationRenderer::AddAnnotation(mTranslationOverlay.GetPointer(), renderer);

  mRotationOverlay = Probe2D::New();
  mRotationOverlay->SetOpacity(1);
  mRotationOverlay->SetVisibility(false);
  mRotationOverlay->SetRadius(50);
  mRotationOverlay->SetColor(0,1,1);
  mitk::ManualPlacementAnnotationRenderer::AddAnnotation(mRotationOverlay.GetPointer(), renderer);
}

Probe2DManager::~Probe2DManager()
{
  std::cout << "Probe 2D manager destructor" << std::endl;
}

void Probe2DManager::UpdatePosition(mitk::Point2D center, mitk::Vector2D dir)
{
  if (mRenderWindow == nullptr)
  {
    cout << "Render window is null in Probe2DManager::UpdatePosition()" << std::endl;
    return;
  }

 // Move all the controls in a coherent way
  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(mRenderWindow->GetVtkRenderWindow());
  mitk::Point2D newCenter;

  mitk::Annotation::Bounds transOvlBounds = mTranslationOverlay->GetBoundsOnDisplay(renderer);
  newCenter[0] = center[0] - transOvlBounds.Size[0]/2;
  newCenter[1] = center[1] - transOvlBounds.Size[1]/2;
  mTranslationOverlay->SetPosition2D(newCenter);


  int* windowSize = mRenderWindow->GetVtkRenderWindow()->GetSize();
  int signs[2] = { -1, 1 };
  for (int i = 0; i < 2; i++)
  {
    mitk::Point2D screenCorner;
    screenCorner[0] = newCenter[0] + signs[i]*windowSize[0]/8;
  }


  //int radius;
  int radius = windowSize[0]<windowSize[1] ? windowSize[0]/8 : windowSize[1]/8;
  mRotationOverlay->SetPosition2D(center);
  //mRotationOverlay->SetCenter(center);
  mRotationOverlay->SetDirection(dir);
  mRotationOverlay->SetRadius(radius);

  renderer->RequestUpdate();
}

void Probe2DManager::ShowControls(bool val)
{
  mTranslationOverlay->SetVisibility(val);
  Update();
}

void Probe2DManager::ShowCircle(bool val)
{
  mRotationOverlay->SetVisibility(val);

  Update();
}

void Probe2DManager::HighlightOn(bool val)
{
  double color = val?0:1;

  mTranslationOverlay->SetColor(1, 1, color);
}

void Probe2DManager::Update()
{
  if (mRenderWindow == nullptr)
  {
    cout << "Render window is null in Probe2DManager::Update()" << std::endl;
    return;
  }

  if (!mRenderWindow->isValid())
  {
    cout << "Render window is invalid!" << std::endl;
    mRenderWindow = nullptr;
    return;
  }

  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(mRenderWindow->GetVtkRenderWindow());
  mTranslationOverlay->Update(renderer);
}
