/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NAVCAS_CASSCENEFILEREADER_H
#define NAVCAS_CASSCENEFILEREADER_H

#include <mitkAbstractFileReader.h>
#include "CASProjectExports.h"

namespace navcas {

class CASProject_EXPORT CASSceneFileReader : public mitk::AbstractFileReader
{

public:

  CASSceneFileReader();

  using AbstractFileReader::Read;
  mitk::DataStorage::SetOfObjects::Pointer Read(mitk::DataStorage &ds) override;

protected:
  std::vector<itk::SmartPointer<mitk::BaseData>> DoRead() override;

private:
  CASSceneFileReader *Clone() const override;
};

}

#endif // NAVCAS_CASSCENEFILEREADER_H
