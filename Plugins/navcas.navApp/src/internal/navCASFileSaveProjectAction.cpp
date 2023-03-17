/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include "internal/QmitkCommonExtPlugin.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

#include <mitkSceneIO.h>
#include <mitkProgressBar.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>

#include <mitkCoreObjectFactory.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferencesService.h>
#include "berryPlatform.h"

#include "navCASFileSaveProjectAction.h"
#include "mitkExampleAppPluginActivator.h"

navCASFileSaveProjectAction::navCASFileSaveProjectAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->Init(window.GetPointer());
}

navCASFileSaveProjectAction::navCASFileSaveProjectAction(berry::IWorkbenchWindow* window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->Init(window);
}

void navCASFileSaveProjectAction::Init(berry::IWorkbenchWindow* window)
{
  m_Window = window;
  this->setText("&Save Project...");
  this->setToolTip("Save navigation or planning scene as a .cas project file");

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}


void navCASFileSaveProjectAction::Run()
{
  try
  {
    /**
     * @brief stores the last path of last saved file
     */
    static QString m_LastPath;

    mitk::IDataStorageReference::Pointer dsRef;

    {
      //ctkPluginContext* context = QmitkCommonExtPlugin::getContext();
      ctkPluginContext* context = mitk::ExampleAppPluginActivator::GetDefault()->GetPluginContext();

      mitk::IDataStorageService* dss = nullptr;
      ctkServiceReference dsServiceRef = context->getServiceReference<mitk::IDataStorageService>();
      if (dsServiceRef)
      {
        dss = context->getService<mitk::IDataStorageService>(dsServiceRef);
      }

      if (!dss)
      {
        QString msg = "IDataStorageService service not available. Unable to open files.";
        MITK_WARN << msg.toStdString();
        QMessageBox::warning(QApplication::activeWindow(), "Unable to open files", msg);
        return;
      }

      // Get the active data storage (or the default one, if none is active)
      dsRef = dss->GetDataStorage();
      context->ungetService(dsServiceRef);
    }

    mitk::DataStorage::Pointer storage = dsRef->GetDataStorage();

    QString dialogTitle = "Save CAS Scene (%1)";
    QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                    dialogTitle.arg(dsRef->GetLabel()),
                                                    m_LastPath,
                                                    "CAS scene files (*.cas)",
                                                    nullptr );

    if (fileName.isEmpty() )
      return;

    // remember the location
    m_LastPath = fileName;

    if ( fileName.right(4) != ".cas" )
      fileName += ".cas";

    mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

    mitk::ProgressBar::GetInstance()->AddStepsToDo(2);

    /* Build list of nodes that should be saved */
    auto isNotHelperObject = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"));//, mitk::BoolProperty::New(true)));
    auto isValid1 = mitk::NodePredicateProperty::New("navCAS.planning.lowResolution",mitk::BoolProperty::New(true));

    // included properties
    auto propertiesIncluded = mitk::NodePredicateOr::New();
    propertiesIncluded->AddPredicate(isNotHelperObject);
    propertiesIncluded->AddPredicate(isValid1);
    propertiesIncluded->AddPredicate(mitk::NodePredicateProperty::New("navCAS.systemSetup.isSetupNode",mitk::BoolProperty::New(true)));
    propertiesIncluded->AddPredicate(mitk::NodePredicateProperty::New("navCAS.navreg.isPrimaryRealNode",mitk::BoolProperty::New(true)));
    propertiesIncluded->Modified();

    // search for null objects
    auto nodesIncluded = storage->GetSubset(propertiesIncluded);
    for (auto it = nodesIncluded->Begin(); it != nodesIncluded->End(); ++it)
    {
      if (it->Value()->GetData() == nullptr)
      {
        it->Value()->SetBoolProperty("navCAS.save.isNull",true);
        cout << "Excluding node " << it->Value()->GetName() << endl;
      }
    }

    // excluded properties
    auto propertiesExcluded = mitk::NodePredicateOr::New();
    auto isGeometry = mitk::NodePredicateDataType::New("GeometryData");
    auto isNull = mitk::NodePredicateProperty::New("navCAS.save.isNull");
    propertiesExcluded->AddPredicate(isGeometry);
    propertiesExcluded->AddPredicate(isNull);
    propertiesExcluded->Modified();

    // totalProperties
    auto totalProperties = mitk::NodePredicateAnd::New();
    totalProperties->AddPredicate(propertiesIncluded);
    totalProperties->AddPredicate(mitk::NodePredicateNot::New(propertiesExcluded));
    totalProperties->Modified();

    // check for null objects and remove from list
    mitk::DataStorage::SetOfObjects::ConstPointer nodesToBeSaved = storage->GetSubset(totalProperties);
    cout << "Trying to save " << nodesToBeSaved->Size() << " nodes:" << endl;


    if (nodesToBeSaved->empty())
    {
      QMessageBox::information(nullptr,
                               "Scene is empty",
                               "Scene could not be saved, because it is empty.",
                               QMessageBox::Ok);
      mitk::ProgressBar::GetInstance()->Reset();
      return;
    }

    if ( !sceneIO->SaveScene( nodesToBeSaved, storage, fileName.toStdString() ) )
    {
      QMessageBox::information(nullptr,
                               "Scene saving",
                               "Scene could not be written completely. Please check the log.",
                               QMessageBox::Ok);

    }
    mitk::ProgressBar::GetInstance()->Progress(2);

    mitk::SceneIO::FailedBaseDataListType::ConstPointer failedNodes = sceneIO->GetFailedNodes();
    if (!failedNodes->empty())
    {
      std::stringstream ss;
      ss << "The following nodes could not be serialized:" << std::endl;
      for ( mitk::SceneIO::FailedBaseDataListType::const_iterator iter = failedNodes->begin();
        iter != failedNodes->end();
        ++iter )
      {
        ss << " - ";
        if ( mitk::BaseData* data =(*iter)->GetData() )
        {
          ss << data->GetNameOfClass();
        }
        else
        {
          ss << "(nullptr)";
        }

        ss << " contained in node '" << (*iter)->GetName() << "'" << std::endl;
      }

      MITK_WARN << ss.str();
    }

    mitk::PropertyList::ConstPointer failedProperties = sceneIO->GetFailedProperties();
    if (!failedProperties->GetMap()->empty())
    {
      std::stringstream ss;
      ss << "The following properties could not be serialized:" << std::endl;
      const mitk::PropertyList::PropertyMap* propmap = failedProperties->GetMap();
      for ( mitk::PropertyList::PropertyMap::const_iterator iter = propmap->begin();
        iter != propmap->end();
        ++iter )
      {
        ss << " - " << iter->second->GetNameOfClass() << " associated to key '" << iter->first << "'" << std::endl;
      }

      MITK_WARN << ss.str();
    }
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Exception caught during scene saving: " << e.what();
  }
}
