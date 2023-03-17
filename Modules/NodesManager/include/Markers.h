/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef Markers_h
#define Markers_h

#include <vector>

#include <mitkCommon.h>
#include <mitkSurface.h>
#include <mitkPointSet.h>
#include <mitkNavigationData.h>

#include <navAPI.h>

#include <NodesManagerExports.h>

using namespace std;

class NodesManager_EXPORT Markers
{

public:

  Markers();
  ~Markers();

	enum MarkerFileType { Temporal, Final };

	/// writes geometry file in relative to marker coordinates (first point set as origin)
  static void WriteGeometryFile(MarkerFileType type, unsigned int serie, const std::vector<mitk::Point3D> &fiducials, mitk::Point3D tip = {0.0});
  /// only updates the tip position in the geometry file
  static void UpdateTemporalProbeTipGeometryFile(MarkerFileType type, unsigned int serie, const mitk::Point3D tip);
  /// creates calibration SQL table for storing acquisitions used for the pivot calibration process
	static QSqlDatabase* CreateCalibrationTable(QString tableName);
	/// stores an acquisition into an already open SQL table
  static void StoreCalibrationData(QSqlDatabase* db, int index, vtkSmartPointer<vtkMatrix4x4> mat, const mitk::Point3D temporalProbePosition);
	/// returns an aproximate position of the probe tip, in absolute camera position coordinates. It rearranges the
	/// fiducials accordingly
	static mitk::Point3D GetProbeTipFromFiducials(unsigned int serie, std::vector<mitk::Point3D> &fiducials);
	/// Read SQL database and store values in vectors
  static bool ReadCalibrationData(QSqlDatabase* db, std::vector<vtkSmartPointer<vtkMatrix4x4>> &matVec, std::vector<mitk::Point3D> &tipVec, std::vector<mitk::NavigationData::Pointer> &ndVec);
private:
  
};

#endif
