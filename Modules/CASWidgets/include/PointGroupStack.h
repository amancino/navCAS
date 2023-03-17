/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef CAS_POINT_GROUP_STACK_H
#define CAS_POINT_GROUP_STACK_H

#include <vector>

#include <mitkPointSet.h>

#include <QObject>

#include "CASWidgetsExports.h"
#include "PointGroup.h"

class CASWidgets_EXPORT PointGroupStack
{

public:

  enum WidgetType {All,SetPoint,RemovePoint,ShowPoint};
  PointGroupStack();
  PointGroupStack(const mitk::PointSet::Pointer);
  ~PointGroupStack();

  PointGroup* at(int pointdId);
  PointGroup* operator[](int pointdId);

  inline int size(){return mStack.size();}

  void pushNewPoint(PointGroup *group);
  PointGroup *pushNewPoint(PointGroup::GroupType type = PointGroup::GroupType::WithoutError);
  void removeLastPoint();
  PointGroup *back();

  int findPointGroupFromSender(QObject* sender, WidgetType = All);

private:

  std::vector<PointGroup*> mStack;

  std::vector<QWidget*> mSetPointList;
  std::vector<QWidget*> mRemovePointList;
  std::vector<QWidget*> mShowPointList;
};


#endif // CAS_POINT_GROUP_STACK_H
