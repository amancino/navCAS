/*===================================================================

navCAS navigation system

@author: Axel Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef CAS_POINT_GROUP
#define CAS_POINT_GROUP

#include "CASWidgetsExports.h"

class QWidget;
class QLabel;
class QPushButton;

struct CASWidgets_EXPORT PointGroup
{
  enum GroupType{WithError,WithoutError};

  PointGroup(int pointNumber, GroupType type = WithError);
  ~PointGroup();

  void SetError(const double err);
  void SetStdDev(const double sd);

  QWidget *     window;
  QLabel*       lblName;
  QPushButton*  pbSet;
  QPushButton*  pbShow;
  QPushButton*  pbRemove;
  QLabel*       lblError;
  QLabel*       lblStdDev;
  double        error;
  double        stdDev;
  int           number;
};


#endif // CAS_IOCOMMANDS_H
