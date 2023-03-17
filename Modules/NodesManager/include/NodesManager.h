/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NodesManager_h
#define NodesManager_h

#include <vector>

#include <QmitkRenderWindow.h>
#include <QObject>

#include <mitkCommon.h>
#include <mitkDataStorage.h>
#include <mitkPointSet.h>

#include <navAPI.h>
#include <NodesManagerExports.h>

#include "ArrowSource.h"
#include "Probe2DManager.h"

/**
  \class NodesManager

  \sa

  This class creates and stores all the probes, markers, fiducials and points used for navigation. Its destructor
  removes everything from the scene, as well.
  If multiple instances of the the class are created the nodes are not generated again, but loaded from the scene
  and replaced.
  It creates a Setup node in the datastorage that stores system setup settings particular to the patient being navigated,
  such as the navigation mode and the geometry id of the markers.
*/

using namespace std;

class NodesManager_EXPORT NodesManager : public QObject
{
  Q_OBJECT

public:

  NodesManager(mitk::DataStorage::Pointer ds);
  virtual ~NodesManager();

public:

  static const std::string SURFACE_NAME;
  static const unsigned int NUMBER_FIDUCIALS[3];

  enum NavigationMode{Traditional, InstrumentTracking, Invalid};

  static void ErrorMessage(const char* err, const char *advice = nullptr);

  // show only the node passed as argument, if it is an image
  void ShowSingleSerie(mitk::DataNode::Pointer node, bool reinit=true);

  mitk::DataNode::Pointer GetSkinNode(const mitk::DataNode::Pointer node);

  /// read system node and configure the navigation mode, and the fixed and moving markers
  void UpdateNavigationConfiguration();


  /// Adds to the datastorage a node containing the mPrimaryPlannedPoints pointset, with a custom mapper
  mitk::DataNode::Pointer CreatePlannedPoints();
  void CreatePrimaryRealPoints();
  void CreateSecondaryPoints();

  /// Assigns the image primary points to the mPrimaryPlannedPoints pointset and displays them.
  /// Returns the number of valid points.
  unsigned int ShowPlannedPoints(bool show=true, mitk::DataNode::Pointer imageNode = nullptr);
  /// Clears the pointset
  void ClearPlannedPoints();
  void SetSecondaryPointsVisibility(bool vis);
  void SetPrimaryRealPoint(unsigned int pos, mitk::Point3D point);
  int InsertNewSecondaryPoint(mitk::Point3D point);
  void RemoveLastSecondaryPoint();
  void ClearSecondaryPoints();
  mitk::PointSet::Pointer GetPrimaryRealPointset();
  mitk::PointSet::Pointer GetPrimaryPlannedPointset();
  mitk::PointSet::Pointer GetSecondaryPointset();




  void LoadMarkersGeometries();

	/// adds correction to current probe tip in probe coordinates
  mitk::Point3D UpdateProbeTip(mitk::Vector3D correction);
  void HideProbe2D();

  /*  Create fiducials, probes, primary and secondary points. By default they should be hidden */
  void CreateMarkers();

  void ShowAllFiducials(bool vis);
  void ShowAllProbes(bool vis);

  /*  Set the reference marker and remove it from other selections, if present */
  void SetReferenceMarker(navAPI::MarkerId type);
  /*  Set the moving marker */
  void SetMovingMarker(navAPI::MarkerId type);

  /*  Set if going to use instrument tracking (requires multiple registrations)  */
  void SetUseMovingMarker(bool use);
  bool GetUseMovingMarker();

  // to do: shouldn't be public: maybe make public only HideMovingMarker()
  /// Moving marker must only be visible if the marker is seen by the camera
  void ShowMovingMarker(bool vis);

  // Store navigation mode in node properties
  void SetNavigationMode(int mode);
  void SetNavigationModeToTraditional();
  void SetNavigationModeToInstrumentTracking();
  // Get navigation mode directly from node properties
  int GetNavigationMode();

  bool GetMovingTMS();

  inline void SetShowProbe(bool show){mShowProbe = show;}

  /// transforms the markers using the matrix, updates the probe (if marker == 2)
  void UpdateRelativeMarker(unsigned int marker, vtkMatrix4x4 *matrix);


  mitk::Point3D GetProbeLastPosition();
  mitk::Vector3D GetProbeDirection();
  inline vector<mitk::Point3D> GetSmallTrackerGeometry(){return mSmallTrackerPos;}
  inline vector<mitk::Point3D> GetMediumTrackerGeometry(){return mMediumTrackerPos;}
  inline vector<mitk::Point3D> GetBigTrackerGeometry(){return mBigTrackerPos;}
  inline vector<mitk::Point3D> GetProbeGeometry(){return mProbePos;}

  void SetUsePlannedPoint(mitk::DataNode::Pointer im,unsigned int pos, bool valid);
  bool GetUsePlannedPoint(mitk::DataNode::Pointer im, unsigned int pos);

  void SetUseRealPoint(unsigned int pos, bool valid);
  bool GetUseRealPoint(unsigned int pos);

  static void StoreTransformInNode(const vtkMatrix4x4* transform, mitk::DataNode::Pointer node);
  static vtkSmartPointer<vtkMatrix4x4> GetTransformFromNode(mitk::DataNode::Pointer node);

  // probe2D drawing
  void SetRenderWindow(QmitkRenderWindow* axial, QmitkRenderWindow* coronal, QmitkRenderWindow* sagittal);

  /// creates the systemsetup node (overrides already existing one)
  void InitializeSetup();
  /// returns the id of the reference marker, as set durring system setup.
  /// returns id "None" if no configuration has been done (or the system node is not present)
  navAPI::MarkerId GetReferenceMarker();
  /// return the id of the moving marker (instrument marker)
  /// returns id "None" if no configuration has been done (or the system node is not present)
  navAPI::MarkerId GetMovingMarker();

  static bool GetShowHelperObjects();
  static void SetShowHelperObjects(bool show);

  inline vtkSmartPointer<vtkMatrix4x4> GetLastMovingMarkerRelativeMatrix(){return mLastMovingMarkerRelativeMatrix;}

private:
  /// updates marker spheres (fiducials) and probe in screen
  void UpdateMarker(unsigned int m,std::vector<mitk::Point3D> pos);

  void UpdateInstrument(vtkTransform* transform);

  /// seeks the system node in the datastorage
  mitk::DataNode::Pointer GetSystemNode();

  // creates the fiducials points of the specified marker
  void GenerateMarkerPointsFromGeometry(navAPI::MarkerId type, mitk::Point3D *pivot=nullptr);

signals:
  void AngleError(double);
  void OffsetError(double);

private:
  mitk::DataStorage::Pointer                mDataStorage;

  // Stored geometries (TODO: turn into vector of vector)
  vector<mitk::Point3D>                     mSmallTrackerPos;
  vector<mitk::Point3D>                     mMediumTrackerPos;
  vector<mitk::Point3D>                     mBigTrackerPos;
  vector<mitk::Point3D>                     mProbePos;

  // 3D arrow probe
  mitk::DataNode::Pointer                   mProbeNode;

  ArrowSource::Pointer                      mProbeSource;

  // surface node that will be moved: surface is a deep copy of the selected instrument node
  mitk::DataNode::Pointer                   mMovingMarkerNode;

  /// stores the geometry of: [0] reference marker, [1] moving marker, [2] probe
  std::vector<navAPI::MarkerId>             mMarkerType;

  // 3 x N (N = 4,4,6)
  vector< vector<mitk::DataNode::Pointer> > mFiducials;

  std::vector<bool>                         mUseRealPoint;

  // Pointsets of planned, real and secondary points
  mitk::PointSet::Pointer                   mPrimaryPlannedPoints;
  mitk::DataNode::Pointer                   mPrimaryPointsetNode;

  mitk::PointSet::Pointer                   mPrimaryRealPoints;
  mitk::PointSet::Pointer                   mSecondaryPoints;

  mitk::DataNode::Pointer                   mSecondaryNode;

  bool                                      mUseMoving;
  bool                                      mShowProbe=true;

  // Store the actual navigation mode, for further requirements before
  // registration or navigation
  NavigationMode                            mNavigationMode=Traditional;

  std::vector<Probe2DManager::Pointer>      mProbe2DManager;
  std::vector<QmitkRenderWindow*>           mRenderWindow;

  vtkSmartPointer<vtkMatrix4x4>             mLastMovingMarkerRelativeMatrix;
};

#endif
