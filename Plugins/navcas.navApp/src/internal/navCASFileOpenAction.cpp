/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <mitkIDataStorageService.h>
#include <mitkNodePredicateProperty.h>
#include <mitkWorkbenchUtil.h>

#include <QmitkIOUtil.h>

#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <QFileDialog>
#include <QSettings>

#include "navCASFileOpenAction.h"
#include "mitkExampleAppPluginActivator.h"
#include "IOCommands.h"


namespace
{
  mitk::DataStorage::Pointer GetDataStorage()
  {
    auto context = mitk::ExampleAppPluginActivator::GetDefault()->GetPluginContext();

    if (nullptr == context)
      return nullptr;

    auto dataStorageServiceReference = context->getServiceReference<mitk::IDataStorageService>();

    if (!dataStorageServiceReference)
      return nullptr;

    auto dataStorageService = context->getService<mitk::IDataStorageService>(dataStorageServiceReference);

    if (nullptr == dataStorageService)
      return nullptr;

    auto dataStorageReference = dataStorageService->GetDataStorage();

    if (dataStorageReference.IsNull())
      return nullptr;

    return dataStorageReference->GetDataStorage();
  }

  mitk::DataNode::Pointer GetFirstSelectedNode()
  {
    auto dataStorage = GetDataStorage();

    if (dataStorage.IsNull())
      return nullptr;

    auto selectedNodes = dataStorage->GetSubset(mitk::NodePredicateProperty::New("selected", mitk::BoolProperty::New(true)));

    if (selectedNodes->empty())
      return nullptr;

    return selectedNodes->front();
  }

  QString GetPathOfFirstSelectedNode()
  {
    auto firstSelectedNode = GetFirstSelectedNode();

    if (firstSelectedNode.IsNull())
      return "";

    auto data = firstSelectedNode->GetData();

    if (nullptr == data)
      return "";

    auto pathProperty = data->GetConstProperty("path");

    if (pathProperty.IsNull())
      return "";

    return QFileInfo(QString::fromStdString(pathProperty->GetValueAsString())).canonicalPath();
  }
}


class navCASFileOpenActionPrivate
{
public:

  void Init(berry::IWorkbenchWindow* window, navCASFileOpenAction* action)
  {
    m_Window = window;
    action->setText("&Open File...");
    action->setToolTip("Open data files (images, surfaces,...)");

    QObject::connect(action, SIGNAL(triggered(bool)), action, SLOT(Run()));
  }

  berry::IPreferences::Pointer GetPreferences() const
  {
    berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
    //berry::IPreferencesService* prefService = mitk::ExampleAppPluginActivator::GetDefault()->GetPreferencesService();
    if (prefService != nullptr)
    {
      return prefService->GetSystemPreferences()->Node("/General");
    }
    return berry::IPreferences::Pointer(nullptr);
  }

  QString GetLastFileOpenPath() const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if (prefs.IsNotNull())
    {
      return prefs->Get("LastFileOpenPath", "");
    }
    return QString();
  }

  void SetLastFileOpenPath(const QString& path) const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if (prefs.IsNotNull())
    {
      prefs->Put("LastFileOpenPath", path);
      prefs->Flush();
    }
  }

  bool GetOpenEditor() const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if (prefs.IsNotNull())
    {
      return prefs->GetBool("OpenEditor", true);
    }
    return true;
  }

  berry::IWorkbenchWindow* m_Window;
};

navCASFileOpenAction::navCASFileOpenAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr)
  , d(new navCASFileOpenActionPrivate)
{
  d->Init(window.GetPointer(), this);
}

navCASFileOpenAction::navCASFileOpenAction(const QIcon& icon, berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr)
  , d(new navCASFileOpenActionPrivate)
{
  d->Init(window.GetPointer(), this);
  setIcon(icon);
}

navCASFileOpenAction::navCASFileOpenAction(const QIcon& icon, berry::IWorkbenchWindow* window)
  : QAction(nullptr), d(new navCASFileOpenActionPrivate)
{
  d->Init(window, this);
  setIcon(icon);
}

navCASFileOpenAction::~navCASFileOpenAction()
{
}

void navCASFileOpenAction::Run()
{
  auto path = GetPathOfFirstSelectedNode();

  if (path.isEmpty())
    path = d->GetLastFileOpenPath();

  std::string fileFilter( "All valid files (*.cas *.mitk *.nii *.nii.gz *.nrrd *.stl *.ply *.vtp);;"
                          "CAS scene (*.cas));;"
                          "MITK scene (*.mitk);;"
                          "Images (*.nii *.nii.gz *nrrd);;"
                          "Meshes (*.stl *.ply *.vtp);;"
                         );

  // Ask the user for a list of files to open
  QStringList fileNames = QFileDialog::getOpenFileNames(nullptr, "Open valid scene files, images, or surfaces...",
                                                        path,
                                                        fileFilter.c_str());

  if (fileNames.empty())
  {
    return;
  }

  // if a single ".cas" scene was loaded, store its location
  if (  (fileNames.size() == 1) &&
        (fileNames[0].right(4) == ".cas") )
  {
    IOCommands::StoreCurrentPatientName(fileNames[0]);
  }

  d->SetLastFileOpenPath(fileNames.front());
  mitk::WorkbenchUtil::LoadFiles(fileNames, berry::IWorkbenchWindow::Pointer(d->m_Window), d->GetOpenEditor());
}
