/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include "navcas_Plugins_Activator.h"

#include "./systemSetup/SystemSetupView.h"
#include "./navigation/NavigationView.h"
#include "./registration/NavRegView.h"

using namespace mitk;

ctkPluginContext* PluginActivator::m_context = nullptr;
PluginActivator* PluginActivator::m_Instance = nullptr;

PluginActivator::PluginActivator()
{
  m_Instance = this;
}

PluginActivator::~PluginActivator()
{
  m_Instance = nullptr;
}

void PluginActivator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(SystemSetupView, context)
  BERRY_REGISTER_EXTENSION_CLASS(NavigationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(NavRegView, context)
  
  this->m_context = context;
}

void PluginActivator::stop(ctkPluginContext *)
{
  this->m_context = nullptr;
}

PluginActivator* PluginActivator::getDefault()
{
  return m_Instance;
}

ctkPluginContext*PluginActivator::getContext()
{
  return m_context;
}
