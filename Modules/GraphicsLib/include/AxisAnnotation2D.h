/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef AXIS_ANNOTATION_2D_H
#define AXIS_ANNOTATION_2D_H

#include <mitkVtkAnnotation2D.h>
#include <mitkLocalStorageHandler.h>
#include <mitkAnnotation.h>
#include <vtkSmartPointer.h>
#include <vtkAxisActor2D.h>

class vtkPolyData;
class vtkActor2D;
class vtkPolyDataMapper2D;

/** \brief Displays PolyDatas as overlays */
class AxisAnnotation2D : public mitk::VtkAnnotation2D {
public:

  mitkClassMacro(AxisAnnotation2D, mitk::VtkAnnotation2D);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  class LocalStorage : public mitk::Annotation::BaseLocalStorage
  {
  public:
    /** \brief Actor of a 2D render window. */
    //vtkSmartPointer<vtkAxisActor2D>  m_LineActor;
    vtkSmartPointer<vtkActor2D>  m_LineActor;
    vtkSmartPointer<vtkPolyData> m_Line;
    vtkSmartPointer<vtkPolyDataMapper2D> m_LineMapper;

    mitk::Point2D m_P0;
    mitk::Point2D m_P1;

    mitk::Point2D m_PrevP0;
    mitk::Point2D m_PrevP1;

    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastUpdateTime;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();
  };



  void SetLinePoints(mitk::Point2D point0, mitk::Point2D point1, mitk::BaseRenderer *renderer);

  vtkProp* GetInternalVtkProp(mitk::BaseRenderer* renderer)
  {
      return GetVtkProp(renderer);
  }

protected:

  /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
  mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

  virtual vtkActor2D* GetVtkActor2D(mitk::BaseRenderer *renderer) const;
  void UpdateVtkAnnotation2D(mitk::BaseRenderer *renderer);

  /** \brief explicit constructor which disallows implicit conversions */
  explicit AxisAnnotation2D();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~AxisAnnotation2D();

private:

  /** \brief copy constructor */
  AxisAnnotation2D(const AxisAnnotation2D&);

  /** \brief assignment operator */
  AxisAnnotation2D& operator=(const AxisAnnotation2D&);
};

#endif // AXIS_ANNOTATION_2D_H


