/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>
#include <algorithm>
#include <string>
#include <random>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkOBBTree.h>

#include "Statistics.h"

using namespace std;

std::vector<std::vector<int> > Statistics::comb(int N, int K)
{
  std::vector<std::vector<int> > combinations;

  std::string bitmask(K, 1); // K leading 1's
  bitmask.resize(N, 0); // N-K trailing 0's

  // print integers and permute bitmask
  do
  {
    std::vector<int> newCombination;
    for (int i = 0; i < N; ++i) // [0..N-1] integers
    {
      if (bitmask[i]) newCombination.push_back(i);
    }
    combinations.push_back(newCombination);
  } while (std::prev_permutation(bitmask.begin(), bitmask.end()));

  return combinations;
}

void Statistics::removeCombinationsWithIds(std::vector<std::vector<int>>& combinations, unsigned int K, const std::vector<int>& excludeIds)
{
  const bool resizeMetrics = (combinations[0].size() - excludeIds.size() == K);

  for (unsigned long c = 0; c<combinations.size(); ++c)
  {
    auto& combination = combinations[c];

    bool remove = false;
    for (unsigned int i=0; i<excludeIds.size(); ++i)
    {
      auto it = std::find(combination.begin(),combination.end(),excludeIds[i]);
      if (it != combination.end())
      {
        if (resizeMetrics)
        {
          combination.erase(it);
        }
        else
        {
          remove = true;
          break;
        }
      }
    }

    if (remove)
    {
      combinations.erase(combinations.begin()+c);
      --c;
    }
  }
}

bool Statistics::getPointCloudMainAxes(const mitk::PointSet* ps, mitk::Vector3D &vx, mitk::Vector3D &vy, mitk::Vector3D &vz)
{
  double OBB_corner[3] = { -1,-1,-1 };
  double OBB_size[3] = { 0,0,0 };

  // turn pointset into polydata
  auto points = vtkSmartPointer<vtkPoints>::New();
  for (int i=0; i<ps->GetSize(); ++i)
  {
    mitk::Point3D point;
    if (ps->GetPointIfExists(i,&point))
      points->InsertNextPoint(point.GetDataPointer());
    else
    {
      MITK_WARN << "Found invalid points in the pointset used to compute the main axes";
      return false;
    }
  }

  // create the tree
  auto obbTree = vtkSmartPointer<vtkOBBTree>::New();
  obbTree->ComputeOBB(points, OBB_corner, vx.GetDataPointer(), vy.GetDataPointer(), vz.GetDataPointer(), OBB_size);

  return true;
}

void Statistics::AddNoiseToPoint(mitk::Point3D &point, double mean, double std)
{
  std::random_device rd;
  std::mt19937 gen(rd());

  // add noise
  for (int i=0; i<3; ++i)
  {
    std::normal_distribution<double> distribution(mean,std);
    double noise = distribution(gen);
    point[i] += noise;
  }
}
