#include <sstream>

#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkFollower.h>
#include <vtkProp3DCollection.h>
#include <vtkPropAssembly.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>

#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkVtkLayerController.h>

#include "LabeledPointSetMapper3D.h"


LabeledPointSetMapper3D::LabeledPointSetMapper3D() : mitk::PointSetVtkMapper3D::PointSetVtkMapper3D()
{
  m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();
}

LabeledPointSetMapper3D::~LabeledPointSetMapper3D()
{

}

void LabeledPointSetMapper3D::ReleaseGraphicsResources(mitk::BaseRenderer* renderer)
{
  mitk::PointSetVtkMapper3D::ReleaseGraphicsResources(renderer);
  m_Assembly->ReleaseGraphicsResources(renderer->GetRenderWindow());
}

vtkProp*  LabeledPointSetMapper3D::GetVtkProp(mitk::BaseRenderer*)
{
  return m_Assembly;
}

void LabeledPointSetMapper3D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
  GetDataNode()->SetFloatProperty("pointsize",3.0);
  mitk::PointSetVtkMapper3D::GenerateDataForRenderer(renderer);

  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer);

  m_Assembly->SetVisibility(visible);

  if (!m_Assembly->GetParts()->IsItemPresent(mitk::PointSetVtkMapper3D::GetVtkProp(renderer)))
    m_Assembly->AddPart(mitk::PointSetVtkMapper3D::GetVtkProp(renderer));

  mitk::VtkMapper::LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
  if (ls->IsGenerateDataRequired(renderer, this, GetDataNode()))
  {
    mitk::PointSet* pointSet = static_cast<mitk::PointSet*>(GetDataNode()->GetData());

    int maxId = -1;
    if (pointSet->GetMaxId() != pointSet->End())
      maxId = static_cast<int>(pointSet->GetMaxId()->Index());

    // Clear assembly and text overlay
    for (unsigned int i=0; i<m_TextOverlay2D.size(); i++)
    {
      if(m_Assembly->GetParts()->IsItemPresent(m_TextOverlay2D[i]->GetInternalVtkProp(renderer)))
        m_Assembly->RemovePart(m_TextOverlay2D[i]->GetInternalVtkProp(renderer));
    }
    m_TextOverlay2D.clear();

    std::string label;
    if (!this->GetDataNode()->GetStringProperty("default label",label))
      label = "P";

    // Add only existing points to assembly
    for (int i=0; i<=maxId; i++)
    {
      m_TextOverlay2D.push_back(TextOverlay2D::New());
      mitk::Point3D point;
      std::stringstream text;

      if (pointSet->GetPointIfExists(i,&point))
      {
        if(!m_Assembly->GetParts()->IsItemPresent(m_TextOverlay2D[i]->GetInternalVtkProp(renderer)))
        {
          m_Assembly->AddPart(m_TextOverlay2D[i]->GetInternalVtkProp(renderer));
        }

        text << label << i+1;
      }
      else
      {
        if(m_Assembly->GetParts()->IsItemPresent(m_TextOverlay2D[i]->GetInternalVtkProp(renderer)))
          m_Assembly->RemovePart(m_TextOverlay2D[i]->GetInternalVtkProp(renderer));
      }

      m_TextOverlay2D[i]->SetText(text.str().c_str());
      m_TextOverlay2D[i]->SetFontSize(18);

      mitk::Point2D dPoint;
      renderer->WorldToDisplay(point,dPoint);

      m_TextOverlay2D[i]->SetPosition2D(dPoint);
    }

    // Only set assembly visible if is not empty
    m_Assembly->SetVisibility(!pointSet->IsEmpty());
  }

  float rgb[3];
  GetDataNode()->GetColor(rgb, renderer);

  for (unsigned int p=0; p<m_TextOverlay2D.size(); p++)
  {
    m_TextOverlay2D[p]->SetColor(rgb);
    m_TextOverlay2D[p]->Update(renderer);
  }

}

void LabeledPointSetMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* base, bool val)
{
  mitk::PointSetVtkMapper3D::SetDefaultProperties(node,base,val);
}


