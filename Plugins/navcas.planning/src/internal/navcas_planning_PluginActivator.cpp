/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#include "navcas_planning_PluginActivator.h"
#include "PlanningView.h"

void navcas_planning_PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(PlanningView, context)
}

void navcas_planning_PluginActivator::stop(ctkPluginContext*)
{
}
