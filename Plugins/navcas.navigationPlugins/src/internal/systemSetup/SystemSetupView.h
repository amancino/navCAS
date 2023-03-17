/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef SystemSetupView_h
#define SystemSetupView_h

#include <navAPI.h>

#include <ui_SystemSetupViewControls.h>
#include "NodesManager.h"
#include "../NavigationPluginBase.h"

class QSqlDatabase;

using namespace std;

class SystemSetupView : public NavigationPluginBase, public mitk::ILifecycleAwarePart
{
  Q_OBJECT
  
public:

  static const std::string VIEW_ID;

  SystemSetupView();
  ~SystemSetupView() override;

private:

  void CreateQtPartControl(QWidget* parent) override;

  // loads system setup node settings from the node's properties
  void LoadPreSettings();

  void SetInstrumentSurface(mitk::DataNode::Pointer node);

  void StartDetectingMarker();

  virtual bool StartNavigation(bool verbose=true, NavigationType=Traditional) override;
  void UpdateCameraStatusLabels(bool isConnected);

  // life cycle aware
  void Activated() override {}
  void Deactivated() override {}
  void Visible() override;
  void Hidden() override;

  void NodeAdded(const mitk::DataNode* node) override;

private slots:

  void OnStartProbeCalibration();

  void OnMultipleMarkersInView(std::vector<navAPI::MarkerId>) override;
  void OnSingleMarkerInView(navAPI::MarkerId) override;
  void OnMultipleOrNullMarkersInView() override;
  void OnValidProbeInView() override;
  void OnInvalidProbeInView() override;

  void OnNavigationModeChanged();
  void OnPatientTrackerChanged();
  void OnInstrumentTrackerChanged();
  void OnSetInstrument();

  void OnValidTemporalProbe();
  void OnAcquireTemporalPosition(unsigned int, vtkMatrix4x4*);

private:
  // Typically a one-liner. Set the focus to the default widget.
  void SetFocus() override;

  // This method is conveniently called whenever the selection of Data Manager
  // items changes.
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& dataNodes) override;

  // Generated from the associated UI file, it encapsulates all the widgets
  // of our view.
  Ui::SystemSetupViewControls						mControls;
  navAPI*                               mCalibrationAPI;

  mitk::DataNode::Pointer               mSelectedInstrument;

	// all acquired temporal probe positions during pivot calibration
	std::vector<mitk::Point3D>									mTemporalProbePositions;
  std::vector<vtkSmartPointer<vtkMatrix4x4> > mTemporalMatrix;
	double																			mError;
	QSqlDatabase*																mCalibrationDB;
};

#endif
