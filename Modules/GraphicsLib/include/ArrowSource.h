/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef ARROWSOURCE_H
#define ARROWSOURCE_H

#include <mitkDataNode.h>

#include <vtkArrowSource.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <GraphicsLibExports.h>
#include <mitkCommon.h>

class GraphicsLib_EXPORT ArrowSource : public itk::LightObject
{ 
public:

  mitkClassMacroItkParent(ArrowSource, itk::LightObject)
  mitkNewMacro3Param(Self,double*,double*,double)

  ArrowSource(double *normal, double *center, double length, double radius=2.5);
  ~ArrowSource() override;

  void SetNormal(double *normal);
  void SetCenter(double *center);

  // to do
  //void SetRadius(double radius);
  //void SetLength();

  void Update();

  vtkSmartPointer<vtkPolyData> GetOutput();

protected:

  vtkSmartPointer<vtkPolyData>      mOutput;
  vtkSmartPointer<vtkPolyData>      mOriginalArrow;
  double                            mNormal[3];
  double                            mLength;
  double                            mCenter[3];
};

#endif // ARROWSOURCE_H
