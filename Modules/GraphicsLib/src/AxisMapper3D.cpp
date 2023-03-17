#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkFollower.h>
#include <vtkProp3DCollection.h>
#include <vtkPropAssembly.h>
#include <vtkProperty.h>

#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkVtkLayerController.h>

#include "AxisMapper3D.h"


AxisMapper3D::AxisMapper3D()
{
  m_LineOverlay2D = AxisAnnotation2D::New();
  m_TextOverlay2D = TextOverlay2D::New();
  m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();
}

AxisMapper3D::~AxisMapper3D()
{
  if (mRenderer.IsNull())
    return;

  m_LineOverlay2D->RemoveFromRenderer(mRenderer,mRenderer->GetVtkRenderer());
}

void AxisMapper3D::ReleaseGraphicsResources(vtkWindow *renWin)
{
    m_Assembly->ReleaseGraphicsResources(renWin);
}

void AxisMapper3D::ReleaseGraphicsResources(mitk::BaseRenderer* renderer)
{
  m_Assembly->ReleaseGraphicsResources(renderer->GetRenderWindow());
}

vtkProp*  AxisMapper3D::GetVtkProp(mitk::BaseRenderer*)
{
  return m_Assembly;
}

void AxisMapper3D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
  mRenderer = renderer;

  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer);
  m_LineOverlay2D->SetVisibility(visible);
  m_Assembly->SetVisibility(visible);
  if (!visible)
  {
    m_LineOverlay2D->Update(renderer);
    return;
  }

  mitk::VtkMapper::LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
  if (ls->IsGenerateDataRequired(renderer, this, GetDataNode()))
  {
    if(!m_Assembly->GetParts()->IsItemPresent(m_LineOverlay2D->GetInternalVtkProp(renderer)) &&
       !m_Assembly->GetParts()->IsItemPresent(m_TextOverlay2D->GetInternalVtkProp(renderer)))
    {
      m_Assembly->AddPart(m_LineOverlay2D->GetInternalVtkProp(renderer));
      m_Assembly->AddPart(m_TextOverlay2D->GetInternalVtkProp(renderer));
      m_Assembly->SetVisibility(true);
    }

    const mitk::PointSet* pointSet = static_cast<const mitk::PointSet*>(GetDataNode()->GetData());
    if (!pointSet->IsEmpty())
    {
      mitk::Point3D p0 = pointSet->GetPoint(0);
      mitk::Point3D p1 = pointSet->GetPoint(1);

      // Update node name
      //GetDataNode()->SetName(ss.str().c_str());

      std::string name = GetDataNode()->GetName();
      m_TextOverlay2D->SetText(name.c_str());
      m_TextOverlay2D->SetFontSize(15);

      mitk::Point2D dp0, dp1;
      renderer->WorldToDisplay(p0,dp0);
      renderer->WorldToDisplay(p1,dp1);

      m_LineOverlay2D->SetLinePoints(dp0.GetDataPointer(), dp1.GetDataPointer(), renderer);

      mitk::Point2D pos;
      pos[0] = (dp0[0] + dp1[0])/2 + 5;
      pos[1] = (dp0[1] + dp1[1])/2;
      m_TextOverlay2D->SetPosition2D(pos);

      //std::cout  << "Pointset (3D): " << p0 << "; " << p1 << std::endl;
      //std::cout  << "Points (2D): " << dp0 << "; " << dp1 << std::endl;
      //std::cout  << "Pos X: " << pos[0] << std::endl;
      //std::cout  << "Pos Y: " << pos[1] << std::endl << std::endl;
    }
  }

  float rgb[3];
  GetDataNode()->GetColor(rgb, renderer);
  m_LineOverlay2D->SetColor(rgb);
  m_TextOverlay2D->SetColor(rgb);

  m_TextOverlay2D->Update(renderer);

  m_LineOverlay2D->RemoveFromRenderer(renderer,renderer->GetVtkRenderer());
  m_LineOverlay2D->Update(renderer);
  m_LineOverlay2D->AddToRenderer(renderer,renderer->GetVtkRenderer());
}

void AxisMapper3D::SetDefaultProperties(mitk::DataNode* , mitk::BaseRenderer* , bool )
{
  /*
  node->AddProperty("selectedcolor", mitk::ColorProperty::New(1.0f, 1.0f, 1.0f), renderer, overwrite);  //white
  node->AddProperty("color", mitk::ColorProperty::New(1.0f, 0.0f, 0.0f), renderer, overwrite);  //red
  node->AddProperty("hover", mitk::BoolProperty::New(false), renderer, overwrite);
  Superclass::SetDefaultProperties(node, renderer, overwrite);
  */
}


