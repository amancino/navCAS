/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef PLANNING_INTERACTOR_H
#define PLANNING_INTERACTOR_H

#include <vector>

#include <QObject>

#include <mitkCommon.h>
#include <mitkDataNode.h>
#include <mitkDataInteractor.h>
#include <mitkDataStorage.h>

#include <InteractorsExports.h>

enum MouseState{None,Moving};

// Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
class Interactors_EXPORT PlanningInteractor : public QObject, public mitk::DataInteractor
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  mitkClassMacro(PlanningInteractor, mitk::DataInteractor)
  mitkNewMacro1Param(Self,const mitk::DataStorage::Pointer)

  PlanningInteractor(const mitk::DataStorage::Pointer ds);
  ~PlanningInteractor() override;

  virtual void SetDataNode(mitk::DataNode* dataNode) override;

  void SetPosition(mitk::Point3D point);
  inline const mitk::Point3D GetPosition(){return mPosition;}

  void ShowArrow(bool show);

protected:

  // Here actions strings from the loaded state machine pattern are mapped to functions of
  // the DataInteractor. These functions are called when an action from the state machine pattern is executed.

  virtual void ConnectActionsAndFunctions() override;

  // This function is called when a DataNode has been set/changed.
  // It is used to initialize the DataNode, e.g. if no PointSet exists yet it is created
  // and added to the DataNode.

  bool MouseClick(const mitk::InteractionEvent *interactionEvent);
  bool StartCameraRotation(const mitk::InteractionEvent *interactionEvent);
  bool CameraRotationFinished(const mitk::InteractionEvent *interactionEvent);

  void DrawAxis();

signals:
  void MoveCrossHair(mitk::Point3D);

private:

  std::vector<mitk::DataNode::Pointer>  mArrowContainer;
  mitk::DataStorage::Pointer            mDataStorage;
  mitk::Point3D                         mPosition;
  bool                                  mIsRotatingCamera;
};

#endif // SPHERE_INTERACTOR
