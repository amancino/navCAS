/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef ViewCommands_H
#define ViewCommands_H

#include <mitkDataStorage.h>
#include <mitkSurface.h>
#include <QmitkRenderWindow.h>
#include <mitkNodePredicateProperty.h>

#include <GraphicsLibExports.h>


class GraphicsLib_EXPORT ViewCommands
{
public:
  ViewCommands();
  ~ViewCommands();

  static void TurnAllSurfacesPickable(const mitk::DataStorage* ds);
  static void GlobalReinit(const mitk::DataStorage* ds, mitk::NodePredicateProperty::Pointer pred = nullptr);

  // a better result is obtained if the datastorage is passed, as none surface will be clipped
  static void Reinit(const mitk::DataNode::Pointer node, const mitk::DataStorage *ds = nullptr);

  static void SetParallelView(QmitkRenderWindow* qmitkRenderWindow);

  static void Set3DCrosshairVisibility(bool vis, mitk::DataStorage::Pointer ds, QmitkRenderWindow* threeD);

  static void AddNodeDescriptor(QString property, QString name, QString icon  = "");
  static void RemoveNodeDescriptor(QString name);
};

#endif // ViewCommands_H
