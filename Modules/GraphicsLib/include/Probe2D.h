/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef PROBE_2D_H
#define PROBE_2D_H

#include <mitkVtkAnnotation2D.h>
#include <vtkActor2D.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkRegularPolygonSource.h>
#include <vtkLineSource.h>

class Probe2D : public mitk::VtkAnnotation2D
{
public:
    mitkClassMacro(Probe2D, mitk::VtkAnnotation2D);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void SetRadius(int radius);
    void SetDirection(mitk::Vector2D dir);

protected:

    Probe2D();
    ~Probe2D();
    vtkActor2D*	GetVtkActor2D(mitk::BaseRenderer *renderer) const;
    void UpdateVtkAnnotation2D(mitk::BaseRenderer *renderer);

    vtkSmartPointer<vtkRegularPolygonSource>    m_circle;
    vtkSmartPointer<vtkLineSource>              m_line;
    vtkSmartPointer<vtkPolyDataMapper2D>        m_mapper;
    vtkSmartPointer<vtkActor2D>                 m_actor;
};

#endif // PROBE_2D_H
