/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NAVCAS_EDITOR_PERSPECTIVE_H_
#define NAVCAS_EDITOR_PERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

class navCASEditorPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  navCASEditorPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer layout) override;

};

#endif /* NAVCAS_EDITOR_PERSPECTIVE_H_ */
