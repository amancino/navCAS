/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NAV_REG_VIEW_H
#define NAV_REG_VIEW_H

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>

#include <mitkPointSet.h>
#include <mitkILifecycleAwarePart.h>
#include <mitkIRenderWindowPartListener.h>

#include <ui_NavRegViewControls.h>

#include <navAPI.h>

#include "SurfaceRefinement.h"
#include "NodesManager.h"
#include "../NavigationPluginBase.h"
#include "IOCommands.h"
#include "PointGroupStack.h"

class QProgressBar;
class QSoundEffect;


class NavRegView : public NavigationPluginBase, public mitk::ILifecycleAwarePart, public mitk::IRenderWindowPartListener
{
  Q_OBJECT
  
public:

  static const std::string VIEW_ID;
  static const double MAX_TOLERATED_PRIMARY_ERROR;
  static const double MAX_TOLERATED_TOTAL_ERROR;

  enum  RegistrationType{None, Patient, Instrument};

  NavRegView();
  ~NavRegView() override;

  void CreateQtPartControl(QWidget* parent) override;

private:
  /// Takes valid primary points and performs a LandmarkBasedTransform registration. This method updates the
  /// mPrimaryRegistrationTransformation and rewrites the mRegistrationTransformation.
  /// It explicitly sets the mSecondaryRegistrationTransformation to the identity.
  /// Returns the registration error.
  double PerformPrimaryRegistration();

  /// Sets the mSecondaryRegistrationTransformation to the obtained result and updates the mRegistrationTransformation.
  void PerformSecondaryRegistration();

  // Stops the navigation/registration process
  void StopRegistration();

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  mitk::Point3D PrimaryToOriginalCoordinates(mitk::Point3D point);

  bool SetCurrentRegistrationSeries(mitk::DataNode::Pointer node, bool changeThroughGUI);

	void CheckRegistrationPoints();

  /// performs registration with the points selected using the probe
  double PerformRegistration();

  void ShowRegistrationPoints(bool show);
  void ShowPrimaryRegistrationPoints(bool show);
  void ShowSecondaryRegistrationPoints(bool show);
  void ClearSecondaryPoints();

  void UpdateAnnotationMessage(string message, bool vis);

  void ConnectSoundEffects();

  void ResetRegistration();

private slots:

  void OnAcceptPrimaryRegistration();
  bool OnCancelPrimaryRegistration();

  void OnAcceptSecondary();
  void OnCancelSecondary();

  /// Accepts current registration (patient or instrument). Transforms the actual surface with
  /// the registration matrix in instrument case, and stores transformation in the node for
  /// the patient case. Modifies node's parameters that might need to be transformed,
  /// and sets "isRegistered" to true. */
  void OnAcceptRegistration();


  void OnStartPatientRegistration();
  void OnStartInstrumentRegistration();
  void OnCancelRegistration();

  void OnAddPoint();

  void OnPerformSurfaceRefinement();
  void OnSurfaceRefinementFinished();
  void OnSurfaceRefinementCanceled();
  void OnUpdateSurfaceRefinementProcess(int steps);

  void OnSetPoint();
  void OnRemovePoint();

  void OnMultipleMarkersInView(std::vector<navAPI::MarkerId>) override { ; }
  void OnSingleMarkerInView(navAPI::MarkerId) override { ; }
	void OnMultipleOrNullMarkersInView() override { ; }
	void OnValidProbeInView() override;
	void OnInvalidProbeInView() override;

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

  bool StartRegistration(mitk::DataNode::Pointer series, navAPI::MarkerId referenceMarker, RegistrationType type);

  void NewAveragedAcquisition(mitk::Point3D,double,int) override;

  Ui::NavRegViewControls                mControls;

  /// selected node in datamanager (either patient or instrument)
  mitk::DataNode::Pointer               mSelectedSeries;
  /// current node used in registration (either patient or instrument)
  mitk::DataNode::Pointer               mRegistrationSeries;
  bool                                  mShowProbe;

  vtkSmartPointer<vtkMatrix4x4>         mPrimaryRegistrationTransformation;
  vtkSmartPointer<vtkMatrix4x4>         mSecondaryRegistrationTransformation;

  RegistrationType                      mRegistrationType;

  // registration results
  mitk::DataNode::Pointer               mPrimaryRealPointsetDisplay;
  mitk::DataNode::Pointer               mSecondaryGreenPointsetDisplay;
  mitk::DataNode::Pointer               mSecondaryRedPointsetDisplay;

  bool                                  mIsNavigating;
  bool                                  mAcceptedRegistration;
  bool                                  mAcceptedPrimaryRegistration;
  bool                                  mSetupIsConfigured;

  SurfaceRefinementThread*              mSurfaceRefinementThread;
  QProgressBar*                         mProgressbar;

  std::vector<mitk::TextAnnotation2D::Pointer>       mRegistrationErrorLabel;

  float                                 mMeanError;

  QSoundEffect*                         mErrorSound;

  int   mCurrentPointId;

  // helper members for storing registration data
  IOCommands::RegistrationPointsData              mRegistrationData;

  PointGroupStack                       mPointGroupStack;
};

#endif
