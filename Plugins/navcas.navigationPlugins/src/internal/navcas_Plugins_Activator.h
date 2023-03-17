/*===================================================================

navCAS navigation system

@author: Axel V. A. Mancino (axel.mancino@gmail.com)

===================================================================*/

#ifndef NAVCAS_PLUGINS_ACTIVATOR_H
#define NAVCAS_PLUGINS_ACTIVATOR_H

// Parent classes
#include <berryAbstractUICTKPlugin.h>

namespace mitk
{
  class PluginActivator : public berry::AbstractUICTKPlugin
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "navcas_navigationPlugins")
    Q_INTERFACES(ctkPluginActivator)

  public:

    PluginActivator();
    ~PluginActivator();

    void start(ctkPluginContext *context) override;
    void stop(ctkPluginContext *context) override;

    static PluginActivator* getDefault();

    static ctkPluginContext* getContext();

  private:

    static ctkPluginContext* m_context;
    static PluginActivator* m_Instance;
  };
}

#endif // NAVCAS_PLUGINS_ACTIVATOR_H
