/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef RegistrationPlanningView_h
#define RegistrationPlanningView_h

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>

#include <mitkILifecycleAwarePart.h>
#include <mitkPointSet.h>

#include <ui_RegistrationPlanningViewControls.h>

#include "PlanningInteractor.h"
#include "NodesManager.h"
#include "PointGroupStack.h"

using namespace std;

class RegistrationPlanningView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  Q_OBJECT
  
public:

  static const std::string VIEW_ID;

  RegistrationPlanningView();
  ~RegistrationPlanningView() override;

  void CreateQtPartControl(QWidget* parent) override;

private:

  // Typically a one-liner. Set the focus to the default widget.
  void SetFocus() override;

  // This method is conveniently called whenever the selection of Data Manager
  // items changes.
  void OnSelectionChanged(
    berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& dataNodes) override;

  void Activated() override;
  void Deactivated() override {}
  void Visible() override {Activated();}
  void Hidden() override;

  void SetRegistrationNode(mitk::DataNode::Pointer node);
  void SetPoint(unsigned int pos, const mitk::Point3D point);
  void RemovePoint(unsigned int pos);
  void ShowPoint(unsigned int p);

private slots:

  void MoveCrossHair(const mitk::Point3D position);

  void OnSetPoint();
  void OnRemovePoint();
  void OnShowPoint();

  void OnStoreCoilRegistrationPoints();
  void OnLoadCoilRegistrationPoints();

  void OnAddPoint();

  void OnRoundPoints();

private:

  void UpdateCoordinatesTable();

  Ui::RegistrationPlanningViewControls  mControls;

  PointGroupStack                       mPointGroupStack;

  // planned primary registration points for display
  mitk::PointSet::Pointer               mRegistrationPoints;
  mitk::DataNode::Pointer               mRegistrationPointsNode;

  // Image to be used in registration
  mitk::DataNode::Pointer               mRegistrationImageNode;

  // Interactor
  PlanningInteractor::Pointer           mInteractor;

  NodesManager*                         mNodesManager;
};

#endif
