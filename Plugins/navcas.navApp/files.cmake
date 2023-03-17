SET(SRC_CPP_FILES
)

SET(INTERNAL_CPP_FILES
  mitkExampleAppPluginActivator.cpp
  QmitkAwesomeApplication.cpp
  QmitkAwesomeWorkbenchAdvisor.cpp
  QmitkAwesomePerspective.cpp
  navCASWorkbenchWindowAdvisor.cpp
	navCASFileSaveProjectAction.cpp
	navCASFileOpenAction.cpp
	navCASEditorPerspective.cpp
)

SET(MOC_H_FILES
  src/internal/QmitkAwesomeApplication.h
  src/internal/mitkExampleAppPluginActivator.h
  src/internal/QmitkAwesomePerspective.h
  src/internal/navCASWorkbenchWindowAdvisor.h
	src/internal/navCASFileSaveProjectAction.h
	src/internal/navCASFileOpenAction.h
	src/internal/navCASEditorPerspective.h
)

SET(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  resources/TMS.xpm
)

SET(QRC_FILES
# uncomment the following line if you want to use Qt resources
  resources/navApp.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

