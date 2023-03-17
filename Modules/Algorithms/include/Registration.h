/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef CAS_Registration_Algorithms_H
#define CAS_Registration_Algorithms_H

#include <vector>

#include <mitkPointSet.h>

#include "AlgorithmsExports.h"

#include <vtkSmartPointer.h>

class vtkMatrix4x4;
class vtkPolyData;

class Algorithms_EXPORT Registration
{

public:

  /// planned points are fixed, real points are moving
  static vtkSmartPointer<vtkMatrix4x4> PerformPairedPointsRegistration(const mitk::PointSet::Pointer planned,
                                                                       const mitk::PointSet::Pointer real,
                                                                       mitk::PointSet::Pointer transformed=nullptr, bool verbose=false);

  static void GetErrorMetricsFromPairedPointRegistration(const mitk::PointSet::Pointer plannedPoints,
                                                           const mitk::PointSet::Pointer transformedPoints,
                                                           std::vector<double> &distances,
                                                           double &meanError, double &std, double &fre, double &fle);

  static void GetAngleAndOffsetErrorFromMatrix(const vtkMatrix4x4* matrix, double &offset, double &angle);

  static double EstimateTREFromMatrix(const vtkMatrix4x4* matrixA, vtkPolyData* pd, const int npoints);

private:

};


#endif // CAS_Registration_Algorithms_H
