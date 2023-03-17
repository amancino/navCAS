#include <vtkPolyDataMapper2D.h>
#include <vtkActor2D.h>
#include <vtkPolyData.h>
#include <vtkProperty2D.h>
#include <vtkAppendPolyData.h>
#include <vtkRegularPolygonSource.h>
#include <vtkLine.h>
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

#include "AxisAnnotation2D.h"

AxisAnnotation2D::AxisAnnotation2D()
{
}

AxisAnnotation2D::~AxisAnnotation2D()
{
}

AxisAnnotation2D::LocalStorage::LocalStorage()
{
  m_LineActor = vtkSmartPointer<vtkActor2D>::New();

  m_Line = vtkSmartPointer<vtkPolyData>::New();
  m_LineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_LineMapper->SetInputData(m_Line);
  m_LineMapper->ScalarVisibilityOff();
  m_LineActor->SetMapper(m_LineMapper);

  m_P0[0] = m_P0[1] = 0;
  m_P1[0] = m_P1[1] = 0;
}

AxisAnnotation2D::LocalStorage::~LocalStorage()
{
}

void AxisAnnotation2D::SetLinePoints(mitk::Point2D point0, mitk::Point2D point1, mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
  if (ls->m_P0 != point0 || ls->m_P1 != point1)
  {
    ls->m_P0 = point0;
    ls->m_P1 = point1;

    Modified();
    //std::cout << "P0: " << point0 << "P1: " << point1 << std::endl;
  }
}

void AxisAnnotation2D::UpdateVtkAnnotation2D(mitk::BaseRenderer *renderer)
{

  LocalStorage* ls = m_LSH.GetLocalStorage(renderer);


  if(ls->IsGenerateDataRequired(renderer, this))
  {

    float color[3] = {1,1,1};
    float opacity = 1.0;
    GetColor(color);//, renderer);
    GetOpacity(opacity);//, renderer);
    vtkProperty2D* property2D = ls->m_LineActor->GetProperty();
    property2D->SetColor(color[0], color[1], color[2]);
    property2D->SetOpacity(opacity);


    if (0 == ls->m_Line->GetNumberOfPoints())
    {
      ls->m_PrevP0 = ls->m_P0;
      ls->m_PrevP1 = ls->m_P1;

      vtkSmartPointer<vtkPolyData> lineData = vtkSmartPointer<vtkPolyData>::New();
      // Draw lines
      vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
      pts->InsertNextPoint(ls->m_P0[0], ls->m_P0[1], 0);
      pts->InsertNextPoint(ls->m_P1[0], ls->m_P1[1], 0);


      vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
      line->GetPointIds()->SetId(0,0); //the second 0 is the index of p0 in the vtkPoints
      line->GetPointIds()->SetId(1,1); //the second 1 is the index of p1 in the vtkPoints

      vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
      lines->InsertNextCell(line);

      // Add the points to the dataset
      lineData->SetPoints(pts);

      // Add the lines to the dataset
      lineData->SetLines(lines);

      // Markers to identify the points (to optimize the update operation)
      vtkSmartPointer<vtkUnsignedCharArray> lineArray = vtkSmartPointer<vtkUnsignedCharArray>::New();
      lineArray->SetNumberOfComponents(1);
      lineArray->SetNumberOfValues(2);
      lineArray->SetValue(0, 0);
      lineArray->SetValue(1, 1);

      lineData->GetPointData()->SetScalars(lineArray);

      ls->m_Line->DeepCopy(lineData);
    }
    else
    {
      mitk::Point2D delta0;
      delta0[0] = ls->m_P0[0] - ls->m_PrevP0[0];
      delta0[1] = ls->m_P0[1] - ls->m_PrevP0[1];

      mitk::Point2D delta1;
      delta1[0] = ls->m_P1[0] - ls->m_PrevP1[0];
      delta1[1] = ls->m_P1[1] - ls->m_PrevP1[1];

      ls->m_PrevP0 = ls->m_P0;
      ls->m_PrevP1 = ls->m_P1;

      vtkUnsignedCharArray* sa = static_cast<vtkUnsignedCharArray*>(ls->m_Line->GetPointData()->GetScalars());

      //std::cout << "Number of tuples: " << sa->GetNumberOfTuples() << std::endl;

      for (vtkIdType id = 0; id < sa->GetNumberOfTuples(); ++id)
      {

        unsigned char v = sa->GetValue(id);
        if (3 == v)
        {
          double p[3];
          ls->m_Line->GetPoints()->GetPoint(id, p);
          ls->m_Line->GetPoints()->SetPoint(id, p[0] + delta1[0], p[1] + delta1[1], 0);
        }
        else if (2 == v)
        {
          double p[3];
          ls->m_Line->GetPoints()->GetPoint(id, p);
          ls->m_Line->GetPoints()->SetPoint(id, p[0] + delta0[0], p[1] + delta0[1], 0);
        }
        else if (1 == v)
        {
          ls->m_Line->GetPoints()->SetPoint(id, ls->m_P1[0], ls->m_P1[1], 0);
        }
        else // 0 == v
        {
          ls->m_Line->GetPoints()->SetPoint(id, ls->m_P0[0], ls->m_P0[1], 0);
        }

      }
    }

    ls->m_Line->Modified();
    ls->UpdateGenerateDataTime();
  }
}

vtkActor2D* AxisAnnotation2D::GetVtkActor2D(mitk::BaseRenderer *renderer) const
{
  LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_LineActor;
}
