/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef AXIS_MAPPER_3D_H
#define AXIS_MAPPER_3D_H

#include <mitkCommon.h>
#include <mitkPointSetVtkMapper3D.h>
#include <mitkPointSet.h>
#include <mitkBaseRenderer.h>
#include <mitkLocalStorageHandler.h>

#include <vtkPointData.h>
#include <vtkVectorText.h>
#include <vtkTransformPolyDataFilter.h>

#include <mitkTextAnnotation2D.h>

#include "GraphicsLibExports.h"
#include "AxisAnnotation2D.h"

class vtkActor;
class vtkPropAssembly;
class vtkFollower;
class vtkPolyDataMapper;

class TextOverlay2D : public mitk::TextAnnotation2D
{
public:
    mitkClassMacro(TextOverlay2D, mitk::TextAnnotation2D)

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    vtkProp* GetInternalVtkProp(mitk::BaseRenderer* renderer)
    {
        return GetVtkProp(renderer);
    }
};

//##Documentation
//## @brief Vtk-based mapper to draw Lines from PointSet
//##
//## @ingroup Mapper
class GraphicsLib_EXPORT AxisMapper3D : public mitk::VtkMapper
{
  public:

    mitkClassMacro(AxisMapper3D, mitk::VtkMapper)

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void ReleaseGraphicsResources(vtkWindow *renWin);
    void ReleaseGraphicsResources(mitk::BaseRenderer* renderer) override;

    virtual void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite);
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

  protected:
     mutable mitk::LocalStorageHandler<mitk::VtkMapper::LocalStorage> m_LSH;

    AxisMapper3D();
    virtual ~AxisMapper3D();

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;

    AxisAnnotation2D::Pointer					m_LineOverlay2D;
    TextOverlay2D::Pointer						m_TextOverlay2D;
    vtkSmartPointer<vtkPropAssembly>  m_Assembly;
    mitk::BaseRenderer::Pointer 			mRenderer;
};

#endif /* AXIS_MAPPER_3D_H */
