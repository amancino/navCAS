/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NAVCAS_FILESAVEPROJECTACTION_H_
#define NAVCAS_FILESAVEPROJECTACTION_H_

#include <QAction>

#include <berrySmartPointer.h>

namespace berry {
struct IWorkbenchWindow;
}

class navCASFileSaveProjectAction : public QAction
{
  Q_OBJECT

public:

  navCASFileSaveProjectAction(berry::SmartPointer<berry::IWorkbenchWindow> window);
  navCASFileSaveProjectAction(berry::IWorkbenchWindow* window);

protected slots:

  void Run();

private:

  void Init(berry::IWorkbenchWindow* window);

  berry::IWorkbenchWindow* m_Window;
};


#endif /*NAVCAS_FILESAVEPROJECTACTION_H_*/
