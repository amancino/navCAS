/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include <usModuleInitialization.h>

#include "navcasCASActivator.h"
#include "navCASSceneFileReader.h"

#include <iostream>

namespace navcas
{
  void CASActivator::Load(us::ModuleContext*)
  {
    std::cout << "CASSceneFileReader loaded " << std::endl;
    m_SceneReader.reset(new CASSceneFileReader);
  }

  void CASActivator::Unload(us::ModuleContext*) {}
}

US_EXPORT_MODULE_ACTIVATOR(navcas::CASActivator)
