/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef REGISTRATIONERRORVISUALIZATION_H
#define REGISTRATIONERRORVISUALIZATION_H

#include <GraphicsLibExports.h>

namespace mitk{
  class DataNode;
}

#include <vtkSmartPointer.h>
class vtkLookupTable;

class GraphicsLib_EXPORT RegistrationErrorVisualization
{
public:
  RegistrationErrorVisualization();

  static vtkSmartPointer<vtkLookupTable> ColorizeNodes(mitk::DataNode* planned, mitk::DataNode* moving, double &mean, double &std);
};

#endif // REGISTRATIONERRORVISUALIZATION_H
