/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef LABELED_POINTSET_MAPPER_3D_H
#define LABELED_POINTSET_MAPPER_3D_H

#include <mitkCommon.h>
#include <mitkPointSetVtkMapper3D.h>
#include <mitkPointSet.h>
#include <mitkBaseRenderer.h>
#include <mitkLocalStorageHandler.h>
#include <mitkTextAnnotation2D.h>

#include <vtkPointData.h>
#include <vtkVectorText.h>
#include <vtkTransformPolyDataFilter.h>

#include "GraphicsLibExports.h"
#include "AxisMapper3D.h"

//##Documentation
//## @brief Vtk-based mapper to draw Lines from PointSet
//##
//## @ingroup Mapper
class GraphicsLib_EXPORT LabeledPointSetMapper3D : public mitk::PointSetVtkMapper3D
{
  public:

    mitkClassMacro(LabeledPointSetMapper3D, mitk::VtkMapper)

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void ReleaseGraphicsResources(mitk::BaseRenderer* renderer) override;

    virtual void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite);
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

  protected:
     mutable mitk::LocalStorageHandler<mitk::VtkMapper::LocalStorage> m_LSH;

    LabeledPointSetMapper3D();
    virtual ~LabeledPointSetMapper3D();

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);

    std::vector<TextOverlay2D::Pointer> m_TextOverlay2D;
    vtkSmartPointer<vtkPropAssembly>    m_Assembly;
};

#endif /* LABELED_POINTSET_MAPPER_3D_H */
