/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef SURFACE_REFINEMENT_H
#define SURFACE_REFINEMENT_H

#include <QThread>

#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>

#include <itkLevenbergMarquardtOptimizer.h>

#include <mitkDataNode.h>
#include <mitkPointSet.h>

#include "AlgorithmsExports.h"

// Observer of surface refinement
class CommandIterationUpdate : public QObject, public itk::Command
{
  Q_OBJECT

public:

  using Self = CommandIterationUpdate;
  using Superclass = itk::Command;
  using Pointer = itk::SmartPointer<Self>;
  itkNewMacro( Self );

protected:
  CommandIterationUpdate() {};

signals:
  void newStep();

public:
  using OptimizerType = itk::LevenbergMarquardtOptimizer;
  using OptimizerPointer = const OptimizerType *;
  void Execute(itk::Object *caller, const itk::EventObject & event) override
  {
    Execute( (const itk::Object *)caller, event);
  }
  void Execute(const itk::Object * object, const itk::EventObject & event) override
  {
    auto optimizer = dynamic_cast< OptimizerPointer >( object );
    if( optimizer == nullptr )
    {
      itkExceptionMacro( "Could not cast optimizer." );
    }
    if( ! itk::IterationEvent().CheckEvent( &event ) )
    {
      return;
    }
    //std::cout << "Value = " << optimizer->GetCachedValue() << std::endl;
    //std::cout << "Position = "  << optimizer->GetCachedCurrentPosition();
    cout << "RMS Error: " << optimizer->GetCachedValue().rms() << std::endl;

    emit newStep();
    //std::cout << std::endl << std::endl;
  }
};

class Algorithms_EXPORT SurfaceRefinementThread : public QThread
{
  Q_OBJECT
  public:
    static vtkSmartPointer<vtkMatrix4x4> GetVtkRegistrationMatrix(itk::Rigid3DTransform<double>::Pointer transform, bool verbose=false);

    inline void SetSurfaceNode(mitk::DataNode::Pointer surfaceNode){mSurfaceNode = surfaceNode;}
    inline void setPointset(mitk::PointSet::Pointer ps){mPointSet = ps;}
    inline bool WasCanceled(){return mCancel;}
    inline double GetError(){return mRMSError;}
    inline mitk::PointSet::Pointer GetGreenPointSet(){return mGreenPointset;}
    inline mitk::PointSet::Pointer GetRedPointSet(){return mRedPointset;}
    inline vtkSmartPointer<vtkMatrix4x4> GetOutputMatrix(){return mSurfaceRefinementMatrix;}

  public slots:
    inline void cancelThread(){mCancel = true;}
    void OnNewStep();
  protected:
    void run();
  signals:
    void percentageCompleted(int);
    void cancelationFinished();
  private:
    int                               mPercentCompleted;
    unsigned int                      mCurrentStep;
    unsigned int                      mMaxIterations;
    bool                              mFilteringOutliers;

    bool                              mCancel;
    mitk::PointSet::Pointer           mPointSet;
    mitk::DataNode::Pointer           mSurfaceNode;
    vtkSmartPointer<vtkMatrix4x4>     mSurfaceRefinementMatrix;

    mitk::PointSet::Pointer           mGreenPointset;
    mitk::PointSet::Pointer           mRedPointset;

    double                            mMeanError;
    double                            mRMSError;

};

#endif
