/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NavigationView_h
#define NavigationView_h

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>

#include <navAPI.h>

#include <mitkPointSet.h>
#include <mitkILifecycleAwarePart.h>

#include <ui_NavigationViewControls.h>
#include "NodesManager.h"
#include "../NavigationPluginBase.h"
#include "ArrowSource.h"
#include "PointGroupStack.h"

class vtkCellLocator;

using namespace std;

class NavigationView : public NavigationPluginBase, public mitk::ILifecycleAwarePart
{
  Q_OBJECT
  
public:
  static const std::string VIEW_ID;

  NavigationView();
  ~NavigationView() override;

  void CreateQtPartControl(QWidget* parent) override;

protected:
  void CheckValidRegistration();
  void HideNonPlanningNodes();
  void RestoreNonPlanningNodes();

  bool StartNavigation(bool verbose=true, NavigationType=Traditional) override;

  // life cycle aware
  void Activated() override {cout << "Navigation activated" << std::endl;}
  void Deactivated() override {cout << "Navigation deactivated" << std::endl;}
  void Visible() override;
  void Hidden() override;

  static void StoreStimulationData(unsigned int index, mitk::Point3D center, mitk::Vector3D dir, mitk::Vector3D viewUp, bool result);

protected slots:

  void OnMultipleMarkersInView(std::vector<navAPI::MarkerId>) override;
  void OnSingleMarkerInView(navAPI::MarkerId) override;
	void OnMultipleOrNullMarkersInView() override;
	void OnValidProbeInView() override;
	void OnInvalidProbeInView() override;

  void OnInformAngleError(double);
  void OnInformOffsetError(double);

  // simulation (for testing)
  void OnStartSimulation();

  void OnExportMeasurements();
  void OnStoreRelativeMatrix();

  void OnStartMeasurements();
  void OnSetMeasurementPoint();
  void OnRemoveMeasurementPoint();

  /// sets the acqusition point with the mean value and sd of the acquisitions
  void NewAveragedAcquisition(const mitk::Point3D point, double sd, int rep) override;

protected:
  // Typically a one-liner. Set the focus to the default widget.
  void SetFocus() override;

  // This method is conveniently called whenever the selection of Data Manager
  // items changes.
  void OnSelectionChanged(
    berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& dataNodes) override;

private:

  Ui::NavigationViewControls										mControls;

  // angle and offset
  std::vector<mitk::TextAnnotation2D::Pointer>  mNavigationErrorText;

  ArrowSource*                  mArrowTest;
  double                        mArrowTolerance;

  // point measurements
  PointGroupStack               mMeasurementPointGroupStack;
  mitk::DataNode::Pointer       mSelectedNode;
  mitk::DataNode::Pointer       mMeasurementPointsNode;
  mitk::PointSet::Pointer       mMeasurementPlanningPointset;
  mitk::PointSet::Pointer       mMeasurementRealPointset;
  mitk::DataNode::Pointer       mMeasurementRealPointsNode;

  bool                          mProbeEnabled;
  int                           mPointId;
};

#endif
