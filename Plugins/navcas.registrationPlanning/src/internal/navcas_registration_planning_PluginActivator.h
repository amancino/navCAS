/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef navcas_registration_planning_PluginActivator_h
#define navcas_registration_planning_PluginActivator_h

#include <ctkPluginActivator.h>

class navcas_registration_planning_PluginActivator
  : public QObject,
    public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "navcas_registration_planning")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);
};

#endif
