/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef PROBE2D_MANAGER_H
#define PROBE2D_MANAGER_H

#include <vector>

#include <mitkCommon.h>
#include <mitkTextAnnotation2D.h>

#include <QmitkRenderWindow.h>
#include <GraphicsLibExports.h>

#include "Probe2D.h"


class GraphicsLib_EXPORT Probe2DManager : public itk::LightObject
{
public:
  mitkClassMacroItkParent(Probe2DManager, itk::LightObject)
  mitkNewMacro1Param(Self,QmitkRenderWindow*)

  Probe2DManager(QmitkRenderWindow* renderWindow);
  virtual ~Probe2DManager();

  void UpdatePosition(mitk::Point2D center, mitk::Vector2D dir);
  void ShowControls(bool val);
  void ShowCircle(bool val);
  void HighlightOn(bool val);
  QmitkRenderWindow* GetAssociatedRenderWindow() { return mRenderWindow; }

  inline void SetRenderWindow(QmitkRenderWindow* rw){mRenderWindow = rw;}
  inline QmitkRenderWindow* GetRenderWindow() const {return mRenderWindow;}

protected:

private:

  void Update();

  Probe2D::Pointer                      mRotationOverlay;
  mitk::TextAnnotation2D::Pointer       mTranslationOverlay;
  QmitkRenderWindow*                    mRenderWindow;
};

#endif // PROBE2D_MANAGER_H
