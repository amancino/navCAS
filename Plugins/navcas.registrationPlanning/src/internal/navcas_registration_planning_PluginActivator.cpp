/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include "navcas_registration_planning_PluginActivator.h"
#include "RegistrationPlanningView.h"

void navcas_registration_planning_PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(RegistrationPlanningView, context)
}

void navcas_registration_planning_PluginActivator::stop(ctkPluginContext*)
{
}
