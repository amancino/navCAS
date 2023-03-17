/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef CAS_STATISTICS_H
#define CAS_STATISTICS_H

#include <vector>

#include <mitkPointSet.h>

#include "AlgorithmsExports.h"


class Algorithms_EXPORT Statistics
{

public:

  /// picks all the combinations of K elements from a group of N
  static std::vector<std::vector<int> > comb(int N, int K);

  /// The K value determines if the combinations should be resized to desired length K, or if they should be discarded directly.
  static void removeCombinationsWithIds(std::vector<std::vector<int>>& combinations, unsigned int K, const std::vector<int>& excludeIds);

  static bool getPointCloudMainAxes(const mitk::PointSet* ps, mitk::Vector3D &ax, mitk::Vector3D &ay, mitk::Vector3D &az);

  static void AddNoiseToPoint(mitk::Point3D &point, double mean, double std);

private:

};


#endif // CAS_STATISTICS_H
