/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef SurfaceAdaptation_H
#define SurfaceAdaptation_H

#include <QThread>
#include <QObject>

#include <mitkDataStorage.h>
#include <mitkSurface.h>

#include <GraphicsLibExports.h>


class GraphicsLib_EXPORT SurfaceAdaptation
{
public:
  SurfaceAdaptation();
  ~SurfaceAdaptation();

  static mitk::Surface::Pointer DecimateSurface(mitk::Surface::Pointer surf, double factor, bool forceExtremeDecimation=false);
  static void AttachLowResolutionSurfaces(mitk::DataStorage::Pointer ds);
  static void DeleteLowResolutionSurfaceFromParent(const mitk::DataNode *parent, mitk::DataStorage::Pointer ds);
  static unsigned int AdaptationsRequired(mitk::DataStorage::Pointer ds=nullptr);
  static unsigned int RemoveOrphanNodes(mitk::DataStorage::Pointer ds);
  static const std::string name;
  static mitk::DataNode::Pointer GetLowResolutionNode(mitk::DataStorage::Pointer, mitk::DataNode::Pointer parent);

  // attaches a low resolution node to surface. If surface is already small enough, it creates a duplicate
  static bool AttachLowResolutionSurface(mitk::DataStorage::Pointer ds, mitk::DataNode::Pointer parent);

private:

};



/*
  New Thread to extract brain cortex
*/
class GraphicsLib_EXPORT SurfaceAdaptationThread : public QThread
{
  Q_OBJECT
  public:
    inline void SetDataStorage(mitk::DataStorage::Pointer ds){mDataStorage = ds;}

    bool WasCancelled(){return mCancel;}
    std::vector<mitk::DataNode::Pointer>* GetLowResolutionStack(){return &mLowResolutionStack;}
    std::vector<mitk::DataNode::Pointer>* GetParentStack(){return &mParentStack;}
    void ResetStacks();

  public slots:
    void CancelThread(){mCancel = true;}
  protected:
    void run();

  signals:
    void percentageCompleted(int);
    void cancellationFinished();

  private:
    int                                         mRequiredAdaptations=0;
    bool                                        mCancel=false;
    mitk::DataStorage::Pointer                  mDataStorage=nullptr;
    std::vector<mitk::DataNode::Pointer>        mLowResolutionStack;
    std::vector<mitk::DataNode::Pointer>        mParentStack;
};


#endif // SurfaceAdaptation_H
