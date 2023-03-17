set(CPP_FILES
  src/internal/navcas_registration_planning_PluginActivator.cpp
  src/internal/RegistrationPlanningView.cpp
)

set(UI_FILES
  src/internal/RegistrationPlanningViewControls.ui
)

set(MOC_H_FILES
  src/internal/navcas_registration_planning_PluginActivator.h
  src/internal/RegistrationPlanningView.h
)

# List of resource files that can be used by the plugin system without loading
# the actual plugin. For example, the icon that is typically displayed in the
# plugin view menu at the top of the application window.
set(CACHED_RESOURCE_FILES
  resources/icon.png
  plugin.xml
)

set(QRC_FILES
)
