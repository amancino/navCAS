/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef CASOBJECTFACTORY_H_INCLUDED
#define CASOBJECTFACTORY_H_INCLUDED

#include "CASMapperExports.h"
#include "mitkCoreObjectFactoryBase.h"

namespace navcas {

class CASMapper_EXPORT ObjectFactory : public mitk::CoreObjectFactoryBase
{
public:

  mitkClassMacro(ObjectFactory,CoreObjectFactoryBase)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  ~ObjectFactory();

  virtual mitk::Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);

  virtual void SetDefaultProperties(mitk::DataNode* node);

  virtual std::string GetFileExtensions() override;

  virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();

  virtual std::string GetSaveFileExtensions() override;

  virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();

  DEPRECATED(void RegisterIOFactories());

protected:
  ObjectFactory();
};

}

#endif
