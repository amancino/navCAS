/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <iostream>

#include <QSettings>
#include <QString>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "PointGroupStack.h"


PointGroupStack::PointGroupStack()
{

}

PointGroupStack::~PointGroupStack()
{

}

PointGroupStack::PointGroupStack(const mitk::PointSet::Pointer ps)
{
  for (int i=0; i<ps->GetSize(); i++)
  {
    auto group = pushNewPoint();

    // enable show and remove
    group->pbShow->setEnabled(true);
    group->pbRemove->setEnabled(true);
  }
}


PointGroup* PointGroupStack::operator[](int pointdId)
{
  return mStack[pointdId];
}

PointGroup* PointGroupStack::at(int pointdId)
{
  return mStack[pointdId];
}


int PointGroupStack::findPointGroupFromSender(QObject* sender, WidgetType type)
{
  int pos = -1;

  std::vector<QWidget*>* list = nullptr;

  // find sender according to type passed
  switch (type)
  {
  case SetPoint:
    list = &mSetPointList;
    break;

  case RemovePoint:
    list = &mRemovePointList;
    break;

  case ShowPoint:
    list = &mShowPointList;
    break;

  case All:
    break;

  default:
    break;
  }

  if (list == nullptr)
    return -1;

  auto it = find(list->begin(),list->end(),sender);
  if (it == list->end())
    return -1;

  pos = it-list->begin()+1;

  return pos;
}

void PointGroupStack::pushNewPoint(PointGroup *group)
{
  mStack.push_back(group);

  // add widgets to corresponding search list
  mSetPointList.push_back(group->pbSet);
  mRemovePointList.push_back(group->pbRemove);
  mShowPointList.push_back(group->pbShow);
}

PointGroup* PointGroupStack::pushNewPoint(PointGroup::GroupType type)
{
  // create new window with point name and buttons to add, remove and show
  int pointNumber = mStack.size() + 1;
  PointGroup* group = new PointGroup(pointNumber,type);

  pushNewPoint(group);

  /*
  // store buttons pointers
  mSetPointList.push_back(btnSet);
  mShowPointList.push_back(btnShow);
  mRemovePointList.push_back(btnRemove);
  mNamePointList.push_back(lblPoint);

  // make connections
  connect(btnSet,SIGNAL(clicked()), this, SLOT(OnSetPoint()));
  connect(btnRemove, SIGNAL(clicked()), this, SLOT(OnRemovePoint()));
  connect(btnShow, SIGNAL(clicked()), this, SLOT(OnShowPoint()));

  // append to current gui layout
  mControls.PointsLayout->addWidget(window);
  */

  return group;
}

void PointGroupStack::removeLastPoint()
{
  auto group = mStack.back();
  if (group == nullptr)
    return;

  mStack.pop_back();
  mSetPointList.pop_back();
  mRemovePointList.pop_back();
  mShowPointList.pop_back();

  delete group;
}


PointGroup *PointGroupStack::back()
{
  return mStack.back();
}
