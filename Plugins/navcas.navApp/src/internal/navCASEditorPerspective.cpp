/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/


#include "navCASEditorPerspective.h"
#include "berryIViewLayout.h"

navCASEditorPerspective::navCASEditorPerspective()
{
}

void navCASEditorPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  QString editorArea = layout->GetEditorArea();

  layout->AddView("org.mitk.views.datamanager", berry::IPageLayout::LEFT, 0.3f, editorArea);

  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.datamanager");
  lo->SetCloseable(false);

  layout->AddView("org.mitk.views.imagenavigator",
    berry::IPageLayout::BOTTOM, 0.5f, "org.mitk.views.datamanager");

  berry::IPlaceholderFolderLayout::Pointer bottomFolder = layout->CreatePlaceholderFolder("bottom", berry::IPageLayout::BOTTOM, 0.7f, editorArea);
  bottomFolder->AddPlaceholder("org.blueberry.views.logview");
  bottomFolder->AddPlaceholder("org.mitk.views.modules");

  layout->AddPerspectiveShortcut("org.mitk.mitkworkbench.perspectives.editor");
  layout->AddPerspectiveShortcut("org.mitk.mitkworkbench.perspectives.visualization");
}
