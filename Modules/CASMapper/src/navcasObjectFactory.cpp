/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <mitkPointSet.h>

#include "navcasObjectFactory.h"

#include "mitkCoreObjectFactory.h"


typedef std::multimap<std::string, std::string> MultimapType;

navcas::ObjectFactory::ObjectFactory()
{

}

navcas::ObjectFactory::~ObjectFactory()
{
}

mitk::Mapper::Pointer navcas::ObjectFactory::CreateMapper(mitk::DataNode* /*node*/, MapperSlotId /*id*/)
{
	return nullptr;
}

void navcas::ObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{

  if ( node == nullptr )
  {
    return;
  }

  mitk::DataNode::Pointer nodePointer = node;
}

std::string navcas::ObjectFactory::GetFileExtensions()
{
  return "";
}

mitk::CoreObjectFactoryBase::MultimapType navcas::ObjectFactory::GetFileExtensionsMap()
{
  return mitk::CoreObjectFactoryBase::MultimapType();
}

std::string navcas::ObjectFactory::GetSaveFileExtensions()
{
  return "";
}

mitk::CoreObjectFactoryBase::MultimapType navcas::ObjectFactory::GetSaveFileExtensionsMap()
{
  return mitk::CoreObjectFactoryBase::MultimapType();
}

void navcas::ObjectFactory::RegisterIOFactories()
{
}
