/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NAVCASFILEOPENACTION_H_
#define NAVCASFILEOPENACTION_H_

#include <berryIWorkbenchWindow.h>

// qt
#include <QAction>
#include <QIcon>

class navCASFileOpenActionPrivate;

class navCASFileOpenAction : public QAction
{
  Q_OBJECT

public:

  navCASFileOpenAction(berry::IWorkbenchWindow::Pointer window);
  navCASFileOpenAction(const QIcon& icon, berry::IWorkbenchWindow::Pointer window);
  navCASFileOpenAction(const QIcon& icon, berry::IWorkbenchWindow* window);

  virtual ~navCASFileOpenAction() override;

protected slots:

  virtual void Run();

private:

  const QScopedPointer<navCASFileOpenActionPrivate> d;

};


#endif /*NAVCASFILEOPENACTION_H_*/
