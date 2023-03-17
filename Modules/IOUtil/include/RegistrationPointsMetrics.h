/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef CAS_RegistrationPointsMetrics_H
#define CAS_RegistrationPointsMetrics_H

#include <vector>

#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>
class QString;

#include "IOUtilExports.h"

class IOUtil_EXPORT RegistrationPointsMetrics
{
public:
  RegistrationPointsMetrics() :
    A_markerType(0),
    A_nFiducials(0),
    A_mean(-1),
    A_std(-1),
    A_FRE(-1),
    A_FLE(-1),
    A_volume(-1),
    A_TRE(-1),
    A_offset(-1),
    A_angle(-1),

    B_markerType(0),
    B_nFiducials(0),
    B_mean(-1),
    B_std(-1),
    B_FRE(-1),
    B_FLE(-1),
    B_volume(-1),
    B_TRE(-1),
    B_offset(-1),
    B_angle(-1),

    FLE(0.0),
    offset(-1),
    angle(-1)
  {
  }

  enum MarkerType{NoData=0,FlatSmall,FlatMedium,FlatBig,SphereSmall,SphereMedium,SphereBieg};
  static void ExportMetricsToFile(const std::vector<RegistrationPointsMetrics> &metrics, const QString &filename);
  static const QString GetCombinationsLastPath();

  // model A
  // combination of points used
  std::vector<int>    A_combination;
  int                 A_markerType;
  int                 A_nFiducials;
  vtkSmartPointer<vtkMatrix4x4> A_matrix;
  // distance of each transformed point to the planned one
  std::vector<double> A_dist;
  double              A_mean; // mean distance
  double              A_std;  // std of distance
  double              A_FRE;
  double              A_FLE;
  double              A_volume;
  // estimations comparing against all points
  double              A_TRE;
  double              A_offset;
  double              A_angle;

  // model B
  // combination of points used
  std::vector<int>    B_combination;
  int                 B_markerType;
  int                 B_nFiducials;
  vtkSmartPointer<vtkMatrix4x4> B_matrix;
  // distance of each transformed point to the planned one
  std::vector<double> B_dist;
  double              B_mean;
  double              B_std;
  double              B_FRE;
  double              B_FLE;
  double              B_volume;
  // estimations comparing against all points
  double              B_TRE;
  double              B_offset;
  double              B_angle;

  double              FLE;
  // Disocciated navigation metrics
  double              offset;
  double              angle;
};


#endif // CAS_RegistrationPointsMetrics_H
