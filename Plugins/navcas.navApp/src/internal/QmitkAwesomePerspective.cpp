/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/
#include "QmitkAwesomePerspective.h"
#include "berryIViewLayout.h"

QmitkAwesomePerspective::QmitkAwesomePerspective()
{
}
 
QmitkAwesomePerspective::QmitkAwesomePerspective(const QmitkAwesomePerspective& other)
: QObject()
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

void QmitkAwesomePerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{

  QString editorArea = layout->GetEditorArea();

  layout->AddView("org.mitk.views.datamanager", 
    berry::IPageLayout::LEFT, 0.2f, editorArea);

  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.datamanager");
  lo->SetCloseable(false);

  // add navigation views
  // planning
  layout->AddView("navcas.views.planningview",berry::IPageLayout::RIGHT,0.8f,editorArea);
  layout->GetViewLayout("navcas.views.planningview")->SetCloseable(false);


  //layout->AddView("org.mitk.views.imagenavigator",
  //  berry::IPageLayout::BOTTOM, 0.5f, "org.mitk.views.datamanager");

  //berry::IFolderLayout::Pointer bottomFolder = layout->CreateFolder("bottom", berry::IPageLayout::BOTTOM, 0.7f, editorArea);
  //bottomFolder->AddView("org.mitk.views.propertylistview");
  //bottomFolder->AddView("org.blueberry.views.logview");
}
