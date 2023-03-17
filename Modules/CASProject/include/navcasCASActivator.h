/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NAVCAS_CAS_ACTIVATOR_H
#define NAVCAS_CAS_ACTIVATOR_H

#include <usModuleActivator.h>

#include <mitkIFileReader.h>

#include <memory>

namespace navcas
{
  class CASActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *context) override;
    void Unload(us::ModuleContext *context) override;

  private:
    std::unique_ptr<mitk::IFileReader> m_SceneReader;
  };
}


#endif // NAVCAS_CAS_ACTIVATOR_H
