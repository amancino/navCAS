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

#include "PointGroup.h"

using namespace std;


PointGroup::PointGroup(int pointNumber,GroupType type) :
  lblError(nullptr),
  lblStdDev(nullptr),
  error(-1),
  number(pointNumber)
{
  // create new window with point name and buttons to add, remove and show
  window = new QWidget;
  auto hl = new QHBoxLayout(window);
  hl->setContentsMargins(3,3,3,3);
  hl->setSpacing(3);

  lblName = new QLabel("Point "+QString::number(pointNumber));
  pbSet = new QPushButton("Set",window);
  pbRemove = new QPushButton("Remove",window);
  pbShow = new QPushButton("Show",window);

  hl->addWidget(lblName);
  hl->addWidget(pbSet);
  hl->addWidget(pbRemove);
  hl->addWidget(pbShow);

  if (type == GroupType::WithError)
  {
    lblError = new QLabel("",window);
    lblStdDev = new QLabel("",window);

    hl->addWidget(lblError);
    hl->addWidget(lblStdDev);
  }

  window->setLayout(hl);

  // gui interactions
  pbRemove->setEnabled(false);
  pbShow->setEnabled(false);
}

void PointGroup::SetError(const double err)
{
  error = err;
  lblError->setText(QString::number(error,'g',2)+"mm");
}

void PointGroup::SetStdDev(const double sd)
{
  stdDev = sd;
  lblStdDev->setText("sd = " + QString::number(stdDev,'g',2)+"mm");
}


PointGroup::~PointGroup()
{
  delete lblName;
  delete pbSet;
  delete pbRemove;
  delete pbShow;
  delete window;
}
