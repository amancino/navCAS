/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <vtkRegularPolygonSource.h>
#include <vtkPolyDataMapper2D.h>
#include <mitkBaseRenderer.h>
#include <vtkProperty2D.h>


#include "Probe2D.h"

const double Probe2DWidth = 3.0;

Probe2D::~Probe2D()
{

}

Probe2D::Probe2D()
{
  // Create a circle
  m_circle = vtkSmartPointer<vtkRegularPolygonSource>::New();
  m_circle->SetNumberOfSides(50);
  m_circle->SetRadius(50);
  m_circle->SetCenter(0,0,0);
  m_circle->GeneratePolygonOff();
  m_circle->Update();

  m_line = vtkSmartPointer<vtkLineSource>::New();
  m_line->SetPoint1(0,0,0);
  m_line->SetPoint2(0,0,0);
  m_line->Update();


  // Visualize
  m_mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_mapper->SetInputConnection(m_line->GetOutputPort());
  m_actor = vtkSmartPointer<vtkActor2D>::New();
  m_actor->SetMapper(m_mapper);
  m_actor->GetProperty()->SetLineWidth(Probe2DWidth);
}

void Probe2D::SetRadius(int radius)
{
  m_circle->SetRadius(radius);
  m_circle->Update();
}


void Probe2D::SetDirection(mitk::Vector2D dir)
{
  // display units
  m_line->SetPoint2(dir[0],dir[1],0);
}


vtkActor2D*	Probe2D::GetVtkActor2D(mitk::BaseRenderer *) const
{
  return m_actor;
}

void Probe2D::UpdateVtkAnnotation2D(mitk::BaseRenderer *)
{
  //m_actor->SetPosition(GetPosition2D(renderer)[0],GetPosition2D(renderer)[1]);
  m_actor->SetPosition(GetPosition2D()[0],GetPosition2D()[1]);
  m_actor->Modified();
}
