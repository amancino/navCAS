/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include "navCASSceneFileReader.h"

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkSceneIO.h>
#include <mitkStandaloneDataStorage.h>

namespace navcas
{
  CASSceneFileReader::CASSceneFileReader() : AbstractFileReader()
  {
    mitk::CustomMimeType mimeType(mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".cas.scene");
    mimeType.SetComment("CAS Scene Files");
    mimeType.SetCategory("CAS Scenes");
    mimeType.AddExtension("cas");

    SetDescription("CAS Scene Reader");
    SetMimeType(mimeType);

    RegisterService();
  }

  mitk::DataStorage::SetOfObjects::Pointer CASSceneFileReader::Read(mitk::DataStorage &ds)
  {
    // const DataStorage::SetOfObjects::STLContainerType& oldNodes = ds.GetAll()->CastToSTLConstContainer();
    mitk::DataStorage::SetOfObjects::ConstPointer oldNodes = ds.GetAll();
    mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();
    sceneIO->LoadScene(GetLocalFileName(), &ds, false);
    mitk::DataStorage::SetOfObjects::ConstPointer newNodes = ds.GetAll();

    // Compute the difference
    mitk::DataStorage::SetOfObjects::Pointer result = mitk::DataStorage::SetOfObjects::New();

    unsigned int index = 0;
    for (mitk::DataStorage::SetOfObjects::ConstIterator iter = newNodes->Begin(), iterEnd = newNodes->End(); iter != iterEnd;
         ++iter)
    {
      if (!oldNodes->empty())
      {
        if (std::find(oldNodes->begin(), oldNodes->end(), iter.Value()) == oldNodes->end())
          result->InsertElement(index++, iter.Value());
      }
      else
      {
        result->InsertElement(index++, iter.Value());
      }
    }

    return result;
  }

  std::vector<mitk::BaseData::Pointer> CASSceneFileReader::DoRead()
  {
    //return mitk::AbstractFileReader::Read();

    std::vector<mitk::BaseData::Pointer> result;

    mitk::DataStorage::Pointer ds = mitk::StandaloneDataStorage::New().GetPointer();
    this->Read(*ds);
    mitk::DataStorage::SetOfObjects::ConstPointer dataNodes = ds->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator iter = dataNodes->Begin(), iterEnd = dataNodes->End();
      iter != iterEnd;
      ++iter)
    {
      result.push_back(iter.Value()->GetData());
    }
    return result;
  }

  CASSceneFileReader* CASSceneFileReader::Clone() const { return new CASSceneFileReader(*this); }
}
