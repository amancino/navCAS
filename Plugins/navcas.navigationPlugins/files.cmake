set(CPP_FILES
  src/internal/navcas_Plugins_Activator.cpp
  src/internal/NavigationPluginBase.cpp
  
  src/internal/systemSetup/SystemSetupView.cpp
  src/internal/navigation/NavigationView.cpp
  src/internal/registration/NavRegView.cpp
)

set(UI_FILES
  src/internal/systemSetup/SystemSetupViewControls.ui
  src/internal/navigation/NavigationViewControls.ui
  src/internal/registration/NavRegViewControls.ui
)

set(MOC_H_FILES
  src/internal/navcas_Plugins_Activator.h
  src/internal/NavigationPluginBase.h
  
  src/internal/systemSetup/SystemSetupView.h
  src/internal/navigation/NavigationView.h
  src/internal/registration/NavRegView.h
)

set(CACHED_RESOURCE_FILES
  resources/systemSetup.png
  resources/navigation.png
  resources/registration.png

  resources/add.wav
  resources/remove.wav
  resources/error.wav

  plugin.xml
)
