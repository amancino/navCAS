/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef PlanningView_h
#define PlanningView_h

#include <mitkILifecycleAwarePart.h>

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>

#include <mitkIRenderWindowPartListener.h>

#include <ui_PlanningViewControls.h>

#include "NodesManager.h"
#include "PlanningInteractor.h"
#include "PointGroupStack.h"

class PlanningView : public QmitkAbstractView, public mitk::ILifecycleAwarePart, public mitk::IRenderWindowPartListener
{
  Q_OBJECT
  
public:

  static const std::string VIEW_ID;
  static const std::string SURFACE_NAME;

  PlanningView();
  ~PlanningView() override;

  void CreateQtPartControl(QWidget* parent) override;

  static mitk::DataNode::Pointer GetSkinNode(const mitk::DataStorage::Pointer ds, const mitk::DataNode::Pointer node);

private:

  // Extracts skin from volume, adds it to datastorage and return a pointer to the new node
  mitk::DataNode::Pointer ExtractSkin(mitk::DataNode *parent);

private slots:

  void OnUseNodeChanged();
  void OnSliderChanged();
  void OnSetShow2D();

  void MoveCrossHair(mitk::Point3D position);

  void RequestUpdateAttachments();
  void UpdateAttachments();

  // point measurements plan
  void OnStartPointPlanning();
  void OnAddMeasurementPoint();
  void OnLoadPointPlanning();

  void OnSetMeasurementPoint();
  void OnShowMeasurementPoint();
  void OnRemoveMeasurementPoint();

private:
  // Typically a one-liner. Set the focus to the default widget.
  void SetFocus() override {};

  // This method is conveniently called whenever the selection of Data Manager
  // items changes.
  void OnSelectionChanged(
    berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& dataNodes) override;

  void NodeAdded(const mitk::DataNode *node) override;
  void NodeRemoved(const mitk::DataNode *node) override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  void AttachObservers(mitk::DataNode::Pointer node) const;
  void VisibilityModified(const itk::Object *object, const itk::EventObject &);
  void ColorModified(const itk::Object *, const itk::EventObject &);

  void Activated() override;
  void Deactivated() override {}
  void Visible() override {Activated();}
  void Hidden() override;

  void CheckInteraction();

  Ui::PlanningViewControls          mControls;

  // selected node in datamanager should be visible while all the others are hidden
  mitk::DataNode::Pointer           mSelectedNode;

  mitk::DataNode::Pointer           mPreviewNode;

  std::vector<QmitkRenderWindow*>   mRenderWindows;

  NodesManager*                     mNodesManager;

  PlanningInteractor::Pointer       mInteractor;

  PointGroupStack                   mMeasurementPointGroupStack;
  mitk::PointSet::Pointer           mMeasurementPlanningPointset;
  mitk::DataNode::Pointer           mMeasurementPointsNode;

  int                               mAttachmentCounter;

};

#endif
