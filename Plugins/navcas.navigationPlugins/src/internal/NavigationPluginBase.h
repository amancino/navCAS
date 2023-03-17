/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NavigationPluginBase_h
#define NavigationPluginBase_h

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>

#include <navAPI.h>

#include <mitkPointSet.h>
#include <mitkILifecycleAwarePart.h>

#include "navcas_navigationPlugins_Export.h"

#include "NodesManager.h"

using namespace std;

class NAVIGATION_BASE_EXPORTS NavigationPluginBase : public QmitkAbstractView
{
  Q_OBJECT
  
public:
  NavigationPluginBase();
  virtual ~NavigationPluginBase();

  enum NavigationType{Traditional,Calibration,RegistrationPatient,RegistrationInstrument};

protected:
	// If true, the registration matrix is loaded
  bool IsReadyToNavigate();
  inline bool IsCameraConnected(){return mCameraConnected;}

	void SetMoveCrosshair(bool val) { mMoveCrosshair = val; }
  void Hide3DCrosshair();

protected slots:
  virtual bool StartNavigation(bool verbose=true, NavigationType=Traditional);

  /// stops navigation thread and hides all probes and markers
  void StopNavigation();
  void SilentlyRetryCameraConnection();
  void UpdateRelativeMarker(unsigned int,vtkMatrix4x4* matrix);
  void RenderWindowClosed();
  void CheckValidRenderWindow();

  virtual void OnSingleMarkerInView(navAPI::MarkerId) = 0;
  virtual void OnMultipleMarkersInView(std::vector<navAPI::MarkerId>) = 0;
  virtual void OnMultipleOrNullMarkersInView() = 0;
  virtual void OnValidProbeInView() = 0;
  virtual void OnInvalidProbeInView() = 0;

  virtual void NewAveragedAcquisition(const mitk::Point3D /*point*/, double /*sd*/, int /*rep*/){}

protected:
	void ConnectNavigationActionsAndFunctions();
  void AverageProbeAcquisitions(int repetitions);

protected:

	bool																	mMoveCrosshair;

  navAPI*                               mAPI;
  navAPI::MarkerId                      mSingleMarkerType;
  mitk::Point3D                         mProbeLastPosition;
  mitk::Vector3D                        mProbeDirection;
  NodesManager*                         mNodesManager;
  vtkSmartPointer<vtkMatrix4x4>         mRegistrationTransformation;

private slots:
  void OnAddAcquisition();

private:

  bool                          mCameraConnected;
  bool                          mCancelCameraConection;

  // points average
  std::vector<mitk::Point3D>    mAcquisitionPoints;
  mitk::Point3D                 mCurrentAcquisitionPoint;
  double                        mCurrentAcquisitionPointSd;
  int                           mProbeRepetitions;
};

#endif
